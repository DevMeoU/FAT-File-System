/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Interface cho FAT File System module, cung cấp các hàm để quản lý
 *   và truy cập hệ thống tập tin FAT.
 *********************************************************************/
#ifndef __FAT_DRIVER_H
#define __FAT_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "fat_driver_types.h"
#include "../common/common_types.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* FAT Entry Values */
#define FAT12_MASK       0x0FFF
#define FAT16_MASK       0xFFFF
#define FAT32_MASK       0x0FFFFFFF
#define FAT12_EOC        0x0FF8
#define FAT16_EOC        0xFFF8
#define FAT32_EOC        0x0FFFFFF8
#define FAT_FREE_CLUSTER 0x00000000

/* File Access Modes */
#define FAT_MODE_READ       0x01
#define FAT_MODE_WRITE      0x02
#define FAT_MODE_CREATE     0x04
#define FAT_MODE_APPEND     0x08
#define FAT_MODE_TRUNCATE   0x10

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Initialize FAT driver
 * 
 * @param config Configuration parameters
 * @return STATUS_SUCCESS if successful, error code otherwise
 */
int32_t fat_init(const fat_config_t *config);
int32_t fat_deinit(void);
int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file);
int32_t fat_close(fat_file_t *file);
int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read);
int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written);
int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin);
int32_t fat_stat(const char *path, fat_dir_entry_t *info);
int32_t fat_unlink(const char *path);
int32_t fat_mkdir(const char *path);
int32_t fat_rmdir(const char *path);

/* Internal functions */
int32_t fat_find_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_create_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_write_dir_entry(const fat_dir_entry_t *entry);
uint32_t fat_alloc_cluster(void);
int32_t fat_free_cluster(uint32_t cluster);

#ifdef __cplusplus
}
#endif

#endif /* __FAT_DRIVER_H */

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
