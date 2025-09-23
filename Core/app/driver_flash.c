#include "driver_flash.h"
#include "diskio.h"
#include "fatfs.h"
#include "ffconf.h"
#include <stdio.h>
#include <string.h>

#define LOG_INFO
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
static void flash_erase_impl(uint16_t sector) {
  log_debug("erase sector %d", 0);
  const sfud_flash *flash = sfud_get_device(0);
  sfud_erase(flash, 0, 4096);
}

static void flash_write_impl(uint32_t addr, size_t size, const uint8_t *data) {
  log_debug("write %d byte data to addr %lu", size, addr);
  const sfud_flash *flash = sfud_get_device(0);
  sfud_erase_write(flash, addr, size, data);
}
static void flash_read_impl(uint32_t addr, size_t size, uint8_t *data) {
  log_debug("read %d byte data for addr %lu", size, addr);
  const sfud_flash *flash = sfud_get_device(0);
  sfud_read(flash, addr, size, data);
}

static driver_flash_interface_t flash_instance = {
    .erase = flash_erase_impl,
    .write = flash_write_impl,
    .read = flash_read_impl,
};

const driver_flash_interface_t *driver_flash_get_interface(void) {
  return &flash_instance;
}

static uint8_t work_buf[_MAX_SS]; // 工作缓冲区
void mkfs(void) {

  // 强制初始化磁盘
  log_debug("Initializing disk...");
  disk_initialize(0);

  // 测试磁盘基本操作
  log_debug("Testing disk operations...");
  DSTATUS stat = disk_status(0);
  log_debug("Disk status: %d", stat);

  if (stat == 0) {
    DWORD sector_count;
    DWORD sector_size;
    disk_ioctl(0, GET_SECTOR_COUNT, &sector_count);
    disk_ioctl(0, GET_SECTOR_SIZE, &sector_size);
    log_debug("Sectors: %lu, Sector size: %lu", sector_count, sector_size);

    // 尝试FAT而不是FAT32
    uint8_t res = f_mkfs(USERPath, FM_FAT, 0, work_buf, sizeof(work_buf));
    printf("mkfs result: %d\n", res);
  } else {
    printf("Disk not ready, status: %d\n", stat);
  }
}

FATFS fs;
void mnt(void) {
  uint8_t res = f_mount(&fs, USERPath, 1);
  printf("mount result: %d\n", res);
}

FIL file;
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

void list_files(char *path) {
  printf("file list:\n");
  FILINFO info;
  DIR dir;
  FRESULT res;
  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    while (1) {
      res = f_readdir(&dir, &info);
      if (res != FR_OK || info.fname[0] == 0)
        break;
      printf("%s\n", info.fname);
    }
  }
  f_closedir(&dir);
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
    printf("%s\n", file_content);
    f_close(&file);
  }
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