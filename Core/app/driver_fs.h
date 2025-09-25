#ifndef __DRIVER_FS_H__
#define __DRIVER_FS_H__

#include "sfud.h"
#include <stdio.h>

#include "driver_log.h"

typedef struct {

} driver_fs_interface_t;

const driver_fs_interface_t *driver_fs_get_interface(void);

void safe_init_filesystem(uint8_t force_reinit);
void sync_filesystem(void);
void init_filesystem(void);
void create_directory(char *dir_name);
void create_file(char *file_name, char *file_content);
void list_files(char *path);
void read_file(char *file_name, char *file_content);
void show_partition_info(void);
void show_directory_tree(char *path);
void show_all_file_contents(char *path);
void demo_filesystem(void);
void print_directory_tree(char *path, int depth);
void print_all_file_contents(char *path, int depth);
#endif // __DRIVER_FLASH_H__