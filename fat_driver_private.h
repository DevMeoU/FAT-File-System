#ifndef FAT_DRIVER_PRIVATE_H
#define FAT_DRIVER_PRIVATE_H

#include <stdint.h>
#include "fat_driver.h"

/*
 * FAT Driver Private Definitions
 */
#define FAT_SECTOR_SIZE       512
#define FAT_DIR_ENTRY_SIZE    32
#define FAT_DIR_NAME_LEN      8
#define FAT_DIR_EXT_LEN       3
#define FAT_CACHE_SIZE        16

/* Directory Entry States */
#define FAT_DIR_EMPTY         0x00
#define FAT_DIR_DELETED       0xE5

/* FAT Entry Values */
#define FAT_FREE_CLUSTER      0x0000000

/* FAT Types */
#define FAT_TYPE_12           12
#define FAT_TYPE_16           16
#define FAT_TYPE_32           32

/* File Attributes */
#define FAT_ATTR_READ_ONLY    0x01
#define FAT_ATTR_HIDDEN       0x02
#define FAT_ATTR_SYSTEM       0x04
#define FAT_ATTR_VOLUME_ID    0x08
#define FAT_ATTR_DIRECTORY    0x10
#define FAT_ATTR_ARCHIVE      0x20
#define FAT_ATTR_LONG_NAME    (FAT_ATTR_READ_ONLY | FAT_ATTR_HIDDEN | FAT_ATTR_SYSTEM | FAT_ATTR_VOLUME_ID)

/* End of Chain Markers */
#define FAT12_EOC             0xFF8
#define FAT16_EOC             0xFFF8
#define FAT32_EOC             0x0FFFFFF8

/* FAT Type Masks */
#define FAT12_MASK            0x00000FFF
#define FAT16_MASK            0x0000FFFF
#define FAT32_MASK            0x0FFFFFFF

/* Cache Entry */
typedef struct {
    uint32_t sector;
    uint8_t data[FAT_SECTOR_SIZE];
    bool valid;
    bool dirty;
} fat_cache_entry_t;

typedef struct {
    uint32_t reserved_sectors;
    uint32_t sectors_per_cluster;
    uint32_t root_dir_sectors;
    uint32_t first_data_sector;
    uint32_t total_clusters;
    uint8_t fat_type;
} fat_config_t;

typedef struct {
    fat_config_t config;
    uint32_t fat_start;
    fat_cache_entry_t cache[FAT_CACHE_SIZE];
} fat_context_t;

extern fat_context_t fat_ctx;

/* Utility macros */
#define FAT_EOC(type) ((type) == FAT_TYPE_12 ? FAT12_EOC : \
                      ((type) == FAT_TYPE_16 ? FAT16_EOC : FAT32_EOC))

#define FAT_MASK(type) ((type) == FAT_TYPE_12 ? FAT12_MASK : \
                       ((type) == FAT_TYPE_16 ? FAT16_MASK : FAT32_MASK))

#endif /* FAT_DRIVER_PRIVATE_H */ 