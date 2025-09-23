#ifndef __DRIVER_FLASH_H__
#define __DRIVER_FLASH_H__

#include "sfud.h"


typedef struct {
  void (*erase)(uint16_t sector);
  void (*write)(uint32_t addr, size_t size, const uint8_t *data);
  void (*read)(uint32_t addr, size_t size, uint8_t *data);
 
} driver_flash_interface_t;

const driver_flash_interface_t *driver_flash_get_interface(void);


void umnt(void);
void mkfs(void);
void mnt(void);
void create_file(char *file_name, char *file_content);
void list_files(char *path);
void read_file(char *file_name, char *file_content);
#endif    // __DRIVER_FLASH_H__