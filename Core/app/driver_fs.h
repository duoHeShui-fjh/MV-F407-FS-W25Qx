#ifndef __DRIVER_FS_H__
#define __DRIVER_FS_H__

#include "sfud.h"

typedef struct {

} driver_fs_interface_t;

const driver_fs_interface_t *driver_fs_get_interface(void);

void safe_init_filesystem(uint8_t enable_multi_drives, uint8_t force_reinit);
void init_filesystem(void);
void create_directory(char *dir_name);
void create_file(char *file_name, char *file_content);
void list_files(char *path);
void read_file(char *file_name, char *file_content);
void show_filesystem_info(void);
void show_partition_info(void);
void demo_multi_paths(void);
void print_directory_tree(char *path, int depth);
#endif // __DRIVER_FLASH_H__