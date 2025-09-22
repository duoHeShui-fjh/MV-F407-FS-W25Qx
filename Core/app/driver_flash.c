#include "driver_flash.h"

static void flash_erase_impl(uint16_t sector) {
  printf("erase sector %d\n", 0);
  const sfud_flash *flash = sfud_get_device(0);
  sfud_erase(flash, 0, 4096);
}

static void flash_write_impl(uint32_t addr, size_t size, const uint8_t *data) {
  printf("write %d byte data to addr %lu\n", size, addr);
  const sfud_flash *flash = sfud_get_device(0);
  sfud_erase_write(flash, addr, size, data);
}
static void flash_read_impl(uint32_t addr, size_t size, uint8_t *data) {
  printf("read %d byte data for addr %lu\n", size, addr);
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