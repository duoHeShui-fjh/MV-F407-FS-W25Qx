#include "driver_flash.h"
#include "diskio.h"
#include "fatfs.h"
#include "ffconf.h"
#include <stdio.h>
#include <string.h>

#include "driver_log.h"

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
