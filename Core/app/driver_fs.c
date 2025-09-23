#include "driver_fs.h"
#include "diskio.h"
#include "fatfs.h"
#include "ffconf.h"
#include <string.h>

static driver_fs_interface_t fs_instance = {

};

const driver_fs_interface_t *driver_fs_get_interface(void) {
  return &fs_instance;
}

static uint8_t work_buf[_MAX_SS]; // 工作缓冲区
FATFS fs;                         // 全局文件系统对象
FIL file;                         // 全局文件对象

// =========================== 基本文件操作函数 ===========================

// 同步文件系统缓存
void sync_filesystem(void) {
  log_debug("Syncing filesystem cache");
  f_sync(&file); // 同步文件缓存

  // 刷新文件系统缓存
  FATFS *fs_ptr;
  DWORD free_clust;
  f_getfree(USERPath, &free_clust, &fs_ptr);
}

void create_directory(char *dir_name) {
  log_info("Creating directory: %s", dir_name);
  uint8_t res = f_mkdir(dir_name);
  if (res == FR_OK)
    log_debug("Directory created");
  else if (res == FR_EXIST)
    log_debug("Directory exists");
  else
    log_warn("Create directory failed: %d", res);
}

void create_file(char *file_name, char *file_content) {
  log_info("Creating file: %s", file_name);
  UINT bwritten = 0;
  FIL local_file;
  uint8_t res = f_open(&local_file, file_name, FA_WRITE | FA_CREATE_ALWAYS);

  if (res == FR_OK) {
    log_debug("File opened");
    res = f_write(&local_file, file_content, strlen(file_content), &bwritten);
    if (res == FR_OK)
      log_debug("File written: %u bytes", bwritten);
    else
      log_warn("Write failed: %d", res);

    f_close(&local_file);
  } else
    log_error("Open file failed: %d", res);
}

void read_file(char *file_name, char *file_content) {
  log_info("Reading file: %s", file_name);
  UINT bytes_read = 0;
  uint8_t res = f_open(&file, file_name, FA_READ);

  if (res == FR_OK) {
    log_debug("File opened");
    res = f_read(&file, file_content, 99, &bytes_read);
    if (res == FR_OK) {
      log_debug("Read %u bytes", bytes_read);
      file_content[bytes_read] = '\0';
      printf("%s\r\n", file_content);
    } else
      log_warn("Read failed: %d", res);

    f_close(&file);
  } else
    log_error("Open file failed: %d", res);
}

void list_files(char *path) {
  log_info("Listing files: %s", path);

  // 刷新文件系统缓存以确保能看到所有文件
  FATFS *fs_ptr;
  DWORD free_clust;
  f_getfree(path, &free_clust, &fs_ptr); // 这会刷新缓存

  FILINFO info;
  DIR dir;
  FRESULT res = f_opendir(&dir, path);

  if (res == FR_OK) {
    log_debug("Directory opened");
    while (1) {
      res = f_readdir(&dir, &info);
      if (res != FR_OK || info.fname[0] == 0)
        break;
      printf("%s\r\n", info.fname);
    }
    f_closedir(&dir);
  } else
    log_error("Open directory failed: %d", res);
}

// =========================== 文件系统初始化函数 ===========================

void safe_init_filesystem(uint8_t force_reinit) {
  log_info("Init filesystem [force:%s]", force_reinit ? "yes" : "no");

  // 初始化磁盘
  log_debug("Initializing disk...");
  disk_initialize(0);

  uint8_t mount_res;
  uint8_t should_format = 0;

  if (force_reinit) {
    log_info("Force reinit filesystem");
    should_format = 1;
  } else {
    log_debug("Checking existing filesystem");
    mount_res = f_mount(&fs, USERPath, 1);

    if (mount_res == FR_OK) {
      log_info("Filesystem mounted successfully");
      return;
    } else {
      log_info("Creating new filesystem");
      should_format = 1;
    }
  }

  if (should_format) {
    DSTATUS stat = disk_status(0);
    if (stat != 0) {
      log_error("Disk not ready: %d", stat);
      return;
    }

    uint8_t format_res =
        f_mkfs(USERPath, FM_FAT, 0, work_buf, sizeof(work_buf));
    if (format_res == FR_OK) {
      log_info("Format success");
      mount_res = f_mount(&fs, USERPath, 1);
      if (mount_res == FR_OK) {
        log_debug("Mount success");
      } else {
        log_warn("Mount failed: %d", mount_res);
      }
    } else {
      log_error("Format failed: %d", format_res);
    }
  }
}

// =========================== 高级文件系统操作函数 ===========================

// 递归打印目录结构
void print_directory_tree(char *path, int depth) {
  DIR dir;
  FILINFO info;
  char full_path[256];

  FRESULT res = f_opendir(&dir, path);
  if (res != FR_OK) {
    log_error("Failed to open directory: %s (%d)", path, res);
    return;
  }

  while (1) {
    res = f_readdir(&dir, &info);
    if (res != FR_OK || info.fname[0] == 0)
      break;

    // 打印缩进
    for (int i = 0; i < depth; i++)
      printf("  ");

    // 检查是否是目录
    if (info.fattrib & AM_DIR) {
      printf("[DIR] %s/\r\n", info.fname);

      // 构建完整路径
      if (strcmp(path, USERPath) == 0)
        snprintf(full_path, sizeof(full_path), "/%s", info.fname);
      else
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.fname);

      // 递归遍历子目录
      print_directory_tree(full_path, depth + 1);
    } else {
      // 显示文件大小
      printf("[FILE] %s (%lu bytes)\r\n", info.fname, info.fsize);
    }
  }

  f_closedir(&dir);
}

// 递归打印指定目录下所有文件内容
void print_all_file_contents(char *path, int depth) {
  DIR dir;
  FILINFO info;
  char full_path[48];    // 减小但使用局部变量
  char file_content[24]; // 减小但使用局部变量
  UINT bytes_read = 0;

  // 限制递归深度防止栈溢出
  if (depth > 5) {
    printf("Maximum depth reached, skipping deeper directories\r\n");
    return;
  }

  FRESULT res = f_opendir(&dir, path);
  if (res != FR_OK) {
    log_error("Failed to open directory: %s (%d)", path, res);
    return;
  }

  while (1) {
    res = f_readdir(&dir, &info);
    if (res != FR_OK || info.fname[0] == 0)
      break;

    // 打印缩进
    for (int i = 0; i < depth; i++)
      printf("  ");

    // 检查是否是目录
    if (info.fattrib & AM_DIR) {
      printf("[DIR] %s/\r\n", info.fname);

      // 构建完整路径
      if (strcmp(path, USERPath) == 0)
        snprintf(full_path, sizeof(full_path), "/%s", info.fname);
      else
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.fname);

      printf("Entering directory: %s\r\n", full_path);
      // 递归遍历子目录
      print_all_file_contents(full_path, depth + 1);
      printf("Exiting directory: %s\r\n", full_path);
    } else {
      // 构建完整文件路径
      if (strcmp(path, USERPath) == 0)
        snprintf(full_path, sizeof(full_path), "/%s", info.fname);
      else
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.fname);

      printf("[FILE] %s (%lu bytes):\r\n", info.fname, info.fsize);

      // 跳过过大的文件以避免内存问题
      if (info.fsize > 100) {
        for (int i = 0; i <= depth; i++)
          printf("  ");
        printf("Content: [File too large, skipped]\r\n");
        continue;
      }

      // 打印文件内容
      FIL file;
      res = f_open(&file, full_path, FA_READ);
      if (res == FR_OK) {
        // 限制读取大小
        UINT read_size = (info.fsize > sizeof(file_content) - 1)
                             ? sizeof(file_content) - 1
                             : info.fsize;

        res = f_read(&file, file_content, read_size, &bytes_read);
        if (res == FR_OK) {
          file_content[bytes_read] = '\0';

          for (int i = 0; i <= depth; i++)
            printf("  ");
          printf("Content: %s", file_content);

          if (info.fsize > sizeof(file_content) - 1)
            printf("... (truncated)\r\n");
          else
            printf("\r\n");
        } else {
          for (int i = 0; i <= depth; i++)
            printf("  ");
          printf("Error reading file: %d\r\n", res);
        }
        f_close(&file);
      } else {
        for (int i = 0; i <= depth; i++)
          printf("  ");
        printf("Error opening file: %d\r\n", res);
      }
      // 添加延时
      osDelay(5);
    }
  }

  f_closedir(&dir);
}

// =========================== 演示和信息显示函数 ===========================

void demo_filesystem(void) {
  printf("--- Filesystem Demo ---\r\n");
  // 检查文件系统状态
  printf("Drive %s status = %d\r\n", USERPath, disk_status(0));

  // 创建主要文件
  printf("Creating main files...\r\n");
  create_file("0:/main.txt", "Main application file\r\n");
  create_file("0:/config.txt", "Configuration data\r\n");

  // 创建备份目录和文件
  printf("Creating backup directory...\r\n");
  create_directory("/backup");
  create_directory("/abc");
  create_directory("/1");
  create_directory("/1/2");
  create_directory("/1/2/3");
  create_directory("/1/2/4");
  create_directory("/1/2/4/5");

  create_directory("/1/3");
  create_directory("/1/4");
  create_directory("/1/5");

  // create_file("/backup.txt", "Backup file\r\n");
  // create_file("/backup/data1.txt", "Backup data 1\r\n");
  // create_file("/backup/data2.txt", "Backup data 2\r\n");
  // create_file("/settings.txt", "Settings backup\r\n");
  // create_file("1:/set2.txt", "Settings2 backup\r\n");
  // create_file("2:/set3.txt", "Settings3 backup\r\n");
  // create_file("0:/set4.txt", "Settings4 backup\r\n");
  // create_file("0:/abc/set5.txt", "Settings5 backup\r\n");
  create_file("/1/set6.txt", "Settings6 backup\r\n");
  create_file("/1/2/set7.txt", "Settings7 backup\r\n");
  create_file("/1/2/3/set7.txt", "Settings7 backup\r\n");

  // 创建临时目录和文件
  // printf("Creating temp directories...\r\n");
  // create_directory("/temp");
  // create_directory("/logs");
  // create_file("/temp.txt", "Temporary file\r\n");
  // create_file("/temp/work1.txt", "Temp work file 1\r\n");
  // create_file("/temp/work2.txt", "Temp work file 2\r\n");
  // create_file("/logs/system.log", "System log\r\n");
  // create_file("/logs/error.log", "Error log\r\n");

  sync_filesystem(); // 同步文件系统缓存以确保能看到所有文件
}

void show_partition_info(void) {
  FATFS *fs;
  DWORD fre_clust;

  printf("\r\n=== Partition Information ===\r\n");

  // 获取分区信息
  FRESULT res = f_getfree(USERPath, &fre_clust, &fs);
  if (res == FR_OK) {
    printf("Drive: %s\r\n", USERPath);
    const char *fs_type_name;
    if (fs->fs_type == FS_FAT12) {
      fs_type_name = "FAT12";
    } else if (fs->fs_type == FS_FAT16) {
      fs_type_name = "FAT16";
    } else if (fs->fs_type == FS_FAT32) {
      fs_type_name = "FAT32";
    } else {
      fs_type_name = "Unknown";
    }
    printf("Partition Type: %s\r\n", fs_type_name);

    // 计算各个区域
    DWORD total_clusters = fs->n_fatent - 2;
    DWORD used_clusters = total_clusters - fre_clust;
    DWORD cluster_size_bytes = fs->csize * _MAX_SS;

    printf("\r\n--- Partition Layout ---\r\n");
    printf("Boot Sector:     Sector 0\r\n");
    printf("FAT Tables:      Sector 1 - %lu\r\n", fs->fatbase + fs->fsize - 1);
    printf("Root Directory:  Sector %lu\r\n", fs->dirbase);
    printf("Data Area:       Sector %lu - %lu\r\n", fs->database,
           fs->database + (total_clusters * fs->csize) - 1);

    printf("\r\n--- Space Usage ---\r\n");
    printf("Total Capacity:  %.2f MB (%lu bytes)\r\n",
           (float)(total_clusters * cluster_size_bytes) / (1024 * 1024),
           total_clusters * cluster_size_bytes);
    printf("Used Space:      %.2f MB (%lu bytes)\r\n",
           (float)(used_clusters * cluster_size_bytes) / (1024 * 1024),
           used_clusters * cluster_size_bytes);
    printf("Free Space:      %.2f MB (%lu bytes)\r\n",
           (float)(fre_clust * cluster_size_bytes) / (1024 * 1024),
           fre_clust * cluster_size_bytes);
    printf("Usage:           %.1f%%\r\n",
           ((float)used_clusters / (float)total_clusters) * 100.0f);

    printf("\r\n--- Cluster Information ---\r\n");
    printf("Total Clusters:  %lu\r\n", total_clusters);
    printf("Used Clusters:   %lu\r\n", used_clusters);
    printf("Free Clusters:   %lu\r\n", fre_clust);
    printf("Cluster Size:    %lu bytes (%d sectors)\r\n", cluster_size_bytes,
           fs->csize);

    printf("\r\n--- FAT Information ---\r\n");
    printf("FAT Start:       Sector %lu\r\n", fs->fatbase);
    printf("FAT Size:        %lu sectors per FAT\r\n", fs->fsize);
    printf("Number of FATs:  %d\r\n", fs->n_fats);
    printf("FAT Entries:     %lu\r\n", fs->n_fatent);

  } else
    log_error("Failed to get partition info: %d", res);
}

void show_directory_tree(char *path) {
  printf("===============================\r\n");
  printf("------- Directory Tree --------\r\n");
  // sync_filesystem();

  // 如果传入NULL或空字符串，使用根目录
  if (path == NULL || strlen(path) == 0) {
    path = USERPath;
    printf("Root Directory: (/)\r\n");
  } else {
    printf("Directory: (%s)\r\n", path);
  }

  print_directory_tree(path, 0);
  printf("===============================\r\n");
}

void show_all_file_contents(char *path) {
  printf("===============================\r\n");
  printf("---- All File Contents -------\r\n");

  // 如果传入NULL或空字符串，使用根目录
  if (path == NULL || strlen(path) == 0) {
    path = USERPath;
    printf("Root Directory: (/)\r\n");
  } else {
    printf("Directory: (%s)\r\n", path);
  }

  print_all_file_contents(path, 0);
  printf("===============================\r\n");
}
