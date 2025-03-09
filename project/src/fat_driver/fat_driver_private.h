/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Private header cho FAT File System module, định nghĩa các cấu trúc và
 *   hàm nội bộ chỉ sử dụng trong module.
 *********************************************************************/
#ifndef FAT_DRIVER_PRIVATE_H
#define FAT_DRIVER_PRIVATE_H

#include <stdint.h>
#include <stdbool.h>
#include "../common/common_types.h"
#include "fat_driver_types.h"
#include "fat_driver_errors.h"

/* Các macro hỗ trợ */
#define FAT_EOC(type) ((type) == FAT_TYPE_12 ? FAT12_EOC : \
                     ((type) == FAT_TYPE_16 ? FAT16_EOC : \
                      FAT32_EOC))

#define FAT_MASK(type) ((type) == FAT_TYPE_12 ? FAT12_MASK : \
                       ((type) == FAT_TYPE_16 ? FAT16_MASK : \
                        FAT32_MASK))

/* Storage Info */
typedef struct {
    uint32_t sector_size;     /* Sector size in bytes */
    uint32_t total_sectors;   /* Total number of sectors */
    uint32_t free_sectors;    /* Number of free sectors */
    uint32_t bad_sectors;     /* Number of bad sectors */
} storage_info_t;

/* Global variables */
extern fat_context_t fat_ctx;
extern fat_boot_sector_t boot_sector;
extern fat_type_t fat_type;

/* Public function declarations */
int32_t fat_find_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_create_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_write_dir_entry(const fat_dir_entry_t *entry);
uint32_t fat_alloc_cluster(void);
int32_t fat_free_cluster(uint32_t cluster);
int32_t fat_read_sector(uint32_t sector, uint8_t *buffer);
int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer);
int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster);
int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster);
int32_t fat_find_free_cluster(uint32_t *cluster);
int32_t fat_convert_to_short_name(const char *name, char *short_name);
void fat_get_name(const fat_dir_entry_t *entry, char *name);

/* Private function prototypes */
int32_t calculate_layout(const fat_boot_sector_t *boot_sector);
uint32_t get_fat_size(const fat_boot_sector_t *boot_sector);
int32_t fat_deinit(void);

#endif /* FAT_DRIVER_PRIVATE_H */

/*********************************************************************
 * UUID: 1c9d2f1b-4c4a-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 