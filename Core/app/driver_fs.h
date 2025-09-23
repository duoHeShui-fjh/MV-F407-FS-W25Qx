#ifndef __DRIVER_FS_H__
#define __DRIVER_FS_H__

#include "sfud.h"
#include <stdio.h>

// 日志级别控制
#define LOG_INFO
#define LOG_WARN
#define LOG_ERROR
// #define LOG_DEBUG

// 日志宏定义
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
void show_directory_tree(void);
void demo_filesystem(void);
void print_directory_tree(char *path, int depth);
#endif // __DRIVER_FLASH_H__