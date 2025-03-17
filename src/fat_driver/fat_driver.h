/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Interface cho FAT File System module, cung cấp các hàm để quản lý
 *   và truy cập hệ thống tập tin FAT.
 *********************************************************************/
#ifndef FAT_DRIVER_H
#define FAT_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "../common/common_types.h"
#include "../hal/hal.h"
#include "fat_driver_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Seek Constants */
#define FAT_SEEK_SET 0
#define FAT_SEEK_CUR 1
#define FAT_SEEK_END 2

/* Public Functions */

/* Initialization/Cleanup */
int32_t fat_init(const char* img_path);
int32_t fat_deinit(void);

/* File Operations */
int32_t fat_open(const char* path, uint8_t mode, fat_file_t* file);
int32_t fat_close(fat_file_t* file);
int32_t fat_read(fat_file_t* file, void* buffer, uint32_t size, uint32_t* bytes_read);
int32_t fat_write(fat_file_t* file, const void* buffer, uint32_t size, uint32_t* bytes_written);
int32_t fat_seek(fat_file_t* file, int32_t offset, uint8_t whence);
int32_t fat_unlink(const char* path);

/* Directory Operations */
int32_t fat_mkdir(const char* path);
int32_t fat_rmdir(const char* path);
int32_t fat_opendir(const char* path, fat_dir_t* dir);
int32_t fat_closedir(fat_dir_t* dir);
int32_t fat_readdir(fat_dir_t* dir, fat_dir_entry_t* entry);
int32_t fat_rewinddir(fat_dir_t* dir);

/* File Information */
int32_t fat_stat(const char* path, fat_dir_entry_t* entry);
int32_t fat_fstat(fat_file_t* file, fat_dir_entry_t* entry);
int32_t fat_access(const char* path, uint8_t mode);

/* Volume Operations */
int32_t fat_mount(const char* path, uint32_t mode);
int32_t fat_umount(void);
int32_t fat_sync(void);

#ifdef __cplusplus
}
#endif

#endif /* FAT_DRIVER_H */

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
