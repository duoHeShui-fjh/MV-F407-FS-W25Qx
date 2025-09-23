#include "driver_fs.h"
#include "diskio.h"
#include "fatfs.h"
#include "ffconf.h"
#include <stdio.h>
#include <string.h>

#define LOG_INFO
#define LOG_WARN
#define LOG_ERROR
// #define LOG_DEBUG

// 条件编译的日志级别宏
#ifdef LOG_DEBUG
#define log_debug(format, ...) printf("[d]:" format "\r\n", ##__VA_ARGS__)
#else
#define log_debug(format, ...)
#endif

#ifdef LOG_INFO
#define log_info(format, ...) printf("[i]:" format "\r\n", ##__VA_ARGS__)
#else
#define log_info(format, ...)
#endif

#ifdef LOG_WARN
#define log_warn(format, ...) printf("[w]:" format "\r\n", ##__VA_ARGS__)
#else
#define log_warn(format, ...)
#endif

#ifdef LOG_ERROR
#define log_error(format, ...) printf("[e]:" format "\r\n", ##__VA_ARGS__)
#else
#define log_error(format, ...)
#endif

static driver_fs_interface_t fs_instance = {

};

const driver_fs_interface_t *driver_fs_get_interface(void) {
  return &fs_instance;
}

static uint8_t work_buf[_MAX_SS]; // 工作缓冲区
FATFS fs;                         // 全局文件系统对象

static void init_single_filesystem(FATFS *fs_obj, const char *path,
                                   uint8_t force_reinit) {
  log_debug("Initializing disk for path %s...", path);

  // 对于主驱动器需要初始化磁盘
  if (strcmp(path, USERPath) == 0) {
    disk_initialize(0);
  }

  uint8_t mount_res;
  uint8_t should_format = 0;

  if (force_reinit) {
    log_info("Force reinitializing filesystem at %s...", path);
    printf("Force reinitializing filesystem at %s...\r\n", path);
    should_format = 1;
  } else {
    // 先尝试挂载现有文件系统
    log_info("Checking existing filesystem at %s...", path);
    mount_res = f_mount(fs_obj, path, 1);

    if (mount_res == FR_OK) {
      log_info("Existing filesystem found and mounted successfully at %s",
               path);
      printf("Filesystem at %s already exists - preserving data\r\n", path);
      return;
    } else {
      log_info("No valid filesystem found at %s, creating new one...", path);
      printf("Creating new filesystem at %s...\r\n", path);
      should_format = 1;
    }
  }

  if (should_format) {
    // 测试磁盘状态（仅对主驱动器）
    if (strcmp(path, USERPath) == 0) {
      DSTATUS stat = disk_status(0);
      if (stat != 0) {
        printf("Disk not ready, status: %d\r\n", stat);
        return;
      }
    }

    // 创建新的文件系统
    uint8_t format_res = f_mkfs(path, FM_FAT, 0, work_buf, sizeof(work_buf));
    printf("%s format result: %d\r\n", path, format_res);

    if (format_res == FR_OK) {
      // 重新挂载新创建的文件系统
      mount_res = f_mount(fs_obj, path, 1);
      printf("%s mount result: %d\r\n", path, mount_res);
    }
  }
}

void safe_init_filesystem(uint8_t enable_multi_drives, uint8_t force_reinit) {
  printf("Initializing file system(s) [multi:%s, force:%s]...\r\n",
         enable_multi_drives ? "yes" : "no", force_reinit ? "yes" : "no");

  // 初始化主驱动器 0:
  init_single_filesystem(&fs, USERPath, force_reinit);

  if (enable_multi_drives) {
    // 初始化drive 1:
    printf("Initializing drive 1:...\r\n");
    init_single_filesystem(&USER1FatFS, USER1Path, force_reinit);

    // 初始化drive 2:
    printf("Initializing drive 2:...\r\n");
    init_single_filesystem(&USER2FatFS, USER2Path, force_reinit);

    printf("Multi-filesystem initialization complete\r\n");
  } else {
    printf("Single filesystem initialization complete\r\n");
  }
}

FIL file;
void create_directory(char *dir_name) {
  log_info("create directory %s", dir_name);
  uint8_t res = f_mkdir(dir_name);
  if (res == FR_OK) {
    log_debug("directory created successfully");
  } else if (res == FR_EXIST) {
    log_debug("directory already exists");
  } else {
    log_debug("create directory failed: %d", res);
  }
}

void create_file(char *file_name, char *file_content) {
  int bwritten = 0;
  log_info("create file %s", file_name);
  uint8_t res;
  UNUSED(res);
  res = f_open(&file, file_name, FA_WRITE | FA_CREATE_ALWAYS);
  log_debug("create file result: %d", res);
  res = f_write(&file, file_content, strlen(file_content), (UINT *)&bwritten);
  log_debug("write file result: %d", res);

  f_close(&file);
}

void read_file(char *file_name, char *file_content) {
  uint8_t res;
  UINT bytes_read = 0;
  log_info("show %s content:", file_name);
  res = f_open(&file, file_name, FA_READ);
  log_debug("open file result: %d", res);
  if (res == FR_OK) {
    res = f_read(&file, file_content, 99,
                 &bytes_read); // 读取最多99字节，留1字节给'\0'
    log_debug("read file result: %d, bytes read: %u", res, bytes_read);
    file_content[bytes_read] = '\0'; // 添加字符串结束符
    printf("%s\r\n", file_content);
    f_close(&file);
  }
}

void list_files(char *path) {
  printf("file list:\r\n");
  FILINFO info;
  DIR dir;
  FRESULT res;
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    while (1) {
      res = f_readdir(&dir, &info);
      if (res != FR_OK || info.fname[0] == 0)
        break;
      printf("%s\r\n", info.fname);
    }
  }
  f_closedir(&dir);
}

// 递归打印目录结构
void print_directory_tree(char *path, int depth) {
  DIR dir;
  FILINFO info;
  char full_path[256];

  FRESULT res = f_opendir(&dir, path);
  if (res != FR_OK) {
    printf("Failed to open directory: %s (error: %d)\r\n", path, res);
    return;
  }

  while (1) {
    res = f_readdir(&dir, &info);
    if (res != FR_OK || info.fname[0] == 0)
      break;

    // 打印缩进
    for (int i = 0; i < depth; i++) {
      printf("  ");
    }

    // 检查是否是目录
    if (info.fattrib & AM_DIR) {
      printf("[DIR] %s/\r\n", info.fname);

      // 构建完整路径
      if (strcmp(path, "/") == 0) {
        snprintf(full_path, sizeof(full_path), "/%s", info.fname);
      } else {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, info.fname);
      }

      // 递归遍历子目录
      print_directory_tree(full_path, depth + 1);
    } else {
      // 显示文件大小
      printf("[FILE] %s (%lu bytes)\r\n", info.fname, info.fsize);
    }
  }

  f_closedir(&dir);
}

void demo_multi_paths(void) {
  printf("\r\n=== Multiple Drive Paths Demo ===\r\n");

  // 展示不同的路径
  printf("Available drive paths:\r\n");
  printf("- %s (Primary drive)\r\n", USERPath);
  printf("- %s (Secondary drive - same physical device)\r\n", USER1Path);
  printf("- %s (Tertiary drive - same physical device)\r\n", USER2Path);

  // 检查驱动器状态
  printf("\r\nChecking drive status:\r\n");
  printf("Drive 0: status = %d\r\n", disk_status(0));
  printf("Drive 1: status = %d\r\n", disk_status(1));
  printf("Drive 2: status = %d\r\n", disk_status(2));

  // 在不同路径创建文件演示
  printf("\r\nCreating files on different paths:\r\n");

  // Drive 0: 添加主要文件
  printf("Creating files on drive 0:...\r\n");
  create_file("c:/main.txt", "Main application file on drive c:\r\n");
  create_file("c:/config.txt", "Configuration data for drive c:\r\n");

  // Drive 1: 添加备份文件
  printf("Creating files on drive 1:...\r\n");
  create_directory("1:/backup");
  create_file("1:/backup.txt", "Backup file on drive 1:\r\n");
  create_file("1:/backup/data1.txt", "Backup data 1 on drive 1:\r\n");
  create_file("1:/backup/data2.txt", "Backup data 2 on drive 1:\r\n");
  create_file("1:/settings.txt", "Settings backup on drive 1:\r\n");

  // Drive 2: 添加临时文件
  printf("Creating files on drive 2:...\r\n");
  create_directory("2:/temp");
  create_directory("2:/logs");
  create_file("2:/temp.txt", "Temporary file on drive 2:\r\n");
  create_file("2:/temp/work1.txt", "Temp work file 1 on drive 2:\r\n");
  create_file("2:/temp/work2.txt", "Temp work file 2 on drive 2:\r\n");
  create_file("2:/logs/system.log", "System log on drive 2:\r\n");
  create_file("2:/logs/error.log", "Error log on drive 2:\r\n");

  printf("\r\nListing files on each drive:\r\n");
  printf("Drive 0: files:\r\n");
  list_files("0:/");
  printf("Drive 1: files:\r\n");
  list_files("1:/");
  printf("Drive 2: files:\r\n");
  list_files("2:/");

  printf("\r\nFiles created on multiple logical drives\r\n");
  printf("Note: All drives currently point to the same physical W25Q128\r\n");
  printf("=============================\r\n");
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

  } else {
    printf("Failed to get partition info (error: %d)\r\n", res);
  }

  printf("\r\n--- Files and Directories ---\r\n");
  print_directory_tree("/", 0);
  printf("===============================\r\n");
}

void show_filesystem_info(void) {
  FATFS *filesystem;
  DWORD fre_clust;
  DWORD fre_sect;
  DWORD tot_sect;

  printf("\r\n=== File System Information ===\r\n");

  // 获取文件系统信息
  FRESULT res = f_getfree(USERPath, &fre_clust, &filesystem);
  if (res == FR_OK) {
    tot_sect = (filesystem->n_fatent - 2) * filesystem->csize;
    fre_sect = fre_clust * filesystem->csize;

    printf("Debug info:\r\n");
    printf("- FAT entries (n_fatent): %lu\r\n", filesystem->n_fatent);
    printf("clusters: %lu\r\n", filesystem->n_fatent - 2);
    printf("- Cluster size (csize): %d sectors\r\n", filesystem->csize);

    // 确定文件系统类型
    const char *fs_type_str = "Unknown";
    if (filesystem->fs_type == FS_FAT12) {
      fs_type_str = "FAT12";
    } else if (filesystem->fs_type == FS_FAT16) {
      fs_type_str = "FAT16";
    } else if (filesystem->fs_type == FS_FAT32) {
      fs_type_str = "FAT32";
    }

    printf("File system type: %s\r\n", fs_type_str);
    printf("Cluster size: %d sectors\r\n", filesystem->csize);
    printf("Total sectors: %lu\r\n", tot_sect);
    printf("Free sectors: %lu\r\n", fre_sect);
    printf("Total space: %.2f KB\r\n", (float)(tot_sect * _MAX_SS) / 1024);
    printf("Free space: %.2f KB\r\n", (float)(fre_sect * _MAX_SS) / 1024);
    printf("Used space: %.2f KB\r\n",
           (float)((tot_sect - fre_sect) * _MAX_SS) / 1024);
  } else {
    printf("Failed to get filesystem info (error: %d)\r\n", res);
  }

  printf("\r\n=== Directory Structure ===\r\n");
  print_directory_tree("/", 0); // 暂时注释掉，可能导致卡死
  printf("Directory tree disabled for debugging\r\n");
  printf("=============================\r\n");
}

/*
#define SPI_START() \ (HAL_GPIO_WritePin(W25Qxx_CS_GPIO_Port, W25Qxx_CS_Pin,
GPIO_PIN_RESET))
#define SPI_STOP() \ (HAL_GPIO_WritePin(W25Qxx_CS_GPIO_Port, W25Qxx_CS_Pin,
GPIO_PIN_SET))
 */

/*
static void sfud_delay(void) {
 osDelay(1);
}*/

/*
static sfud_err spi_write_read(const sfud_spi *spi, const uint8_t *write_buf,
                               size_t write_size, uint8_t *read_buf,
                               size_t read_size) {
  sfud_err result = SFUD_SUCCESS;


SPI_START();
if (write_size > 0) {
  HAL_SPI_Transmit(&hspi1, write_buf, write_size, 0xFFFF);
}
if (read_size > 0) {
  HAL_SPI_Receive(&hspi1, read_buf, read_size, 0xFFFF);
}
SPI_STOP();
return result;
}
*/

/*
sfud_err sfud_spi_port_init(sfud_flash *flash) {
  sfud_err result = SFUD_SUCCESS;

  flash->spi.wr = spi_write_read;
  flash->retry.times = 10000;
  flash->retry.delay = sfud_delay;

  return result;
}
*/