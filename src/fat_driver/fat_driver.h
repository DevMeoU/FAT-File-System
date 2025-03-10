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
#include "fat_driver_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Seek Constants */
#define FAT_SEEK_SET 0
#define FAT_SEEK_CUR 1
#define FAT_SEEK_END 2

/* File Operation Modes */
#define FAT_MODE_READ_ONLY  0x01
#define FAT_MODE_WRITE_ONLY 0x02
#define FAT_MODE_READ_WRITE 0x03
#define FAT_MODE_APPEND     0x04
#define FAT_MODE_CREATE     0x08
#define FAT_MODE_TRUNCATE   0x10

/* Public Function Prototypes */
int32_t fat_init(const char* img_path);
int32_t fat_deinit(void);
int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file);
void fat_close(fat_file_t *file);
int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read);
int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written);
int32_t fat_seek(fat_file_t *file, int32_t offset, uint8_t whence);
int32_t fat_unlink(const char *path);
int32_t fat_mkdir(const char *path);
int32_t fat_rmdir(const char *path);

/* Directory Operations */
int32_t fat_opendir(const char *path, fat_dir_t *dir);
int32_t fat_closedir(fat_dir_t *dir);
int32_t fat_readdir(fat_dir_t *dir, fat_entry_t *entry);
int32_t fat_rewinddir(fat_dir_t *dir);

/* File Information */
int32_t fat_stat(const char *path, fat_entry_t *entry);
int32_t fat_fstat(fat_file_t *file, fat_entry_t *entry);
int32_t fat_access(const char *path, uint8_t mode);

/* Volume Operations */
int32_t fat_mount(const char *path, uint32_t mode);
int32_t fat_umount(void);
int32_t fat_sync(void);

/* Private Function Prototypes */
int32_t fat_read_boot_sector(fat_driver_private_t *driver);
int32_t fat_read_fat_table(fat_driver_private_t *driver);
int32_t fat_read_root_dir(fat_driver_private_t *driver);
int32_t fat_get_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t *value);
int32_t fat_set_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t value);
int32_t fat_get_next_cluster(fat_driver_private_t *driver, uint32_t cluster, uint32_t *next_cluster);
int32_t fat_is_eof_cluster(fat_driver_private_t *driver, uint32_t cluster);
int32_t fat_is_bad_cluster(fat_driver_private_t *driver, uint32_t cluster);
int32_t fat_is_free_cluster(uint32_t cluster);
int32_t fat_get_cluster_offset(fat_driver_private_t *driver, uint32_t cluster);
int32_t fat_get_root_dir_offset(fat_driver_private_t *driver, uint32_t entry_index);
int32_t fat_find_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_create_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_write_dir_entry(const fat_dir_entry_t *entry);
int32_t fat_convert_to_short_name(const char *long_name, char *short_name);
uint16_t fat_get_date(void);
uint16_t fat_get_time(void);
void fat_get_name(const fat_dir_entry_t *entry, char *name);

#ifdef __cplusplus
}
#endif

#endif /* FAT_DRIVER_H */

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
