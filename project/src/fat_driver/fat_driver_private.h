#ifndef FAT_DRIVER_PRIVATE_H
#define FAT_DRIVER_PRIVATE_H

#include "fat_driver_types.h"
#include "../hal/hal.h"

/**
 * Constants and macros for FAT Driver
 */
#define FAT12_EOC 0xFFF      /**< End of cluster chain for FAT12 */
#define FAT16_EOC 0xFFFF     /**< End of cluster chain for FAT16 */
#define FAT32_EOC 0x0FFFFFFF /**< End of cluster chain for FAT32 */

#define FAT_ATTR_READ_ONLY  0x01 /**< Read-only attribute */
#define FAT_ATTR_HIDDEN     0x02 /**< Hidden attribute */
#define FAT_ATTR_SYSTEM     0x04 /**< System attribute */
#define FAT_ATTR_VOLUME_ID  0x08 /**< Volume ID attribute */
#define FAT_ATTR_DIRECTORY  0x10 /**< Directory attribute */
#define FAT_ATTR_ARCHIVE    0x20 /**< Archive attribute */
#define FAT_ATTR_LFN        0x0F /**< Long File Name attribute */

/**
 * Structure for a FAT directory entry
 */
typedef struct __attribute__((packed)) {
    uint8_t name[8];           /**< File name (8 characters) */
    uint8_t ext[3];            /**< File extension (3 characters) */
    uint8_t attributes;        /**< File attributes */
    uint8_t reserved;          /**< Reserved for Windows NT */
    uint8_t create_time_tenth; /**< Time created (tenths of a second) */
    uint16_t create_time;      /**< Time created */
    uint16_t create_date;      /**< Date created */
    uint16_t last_access_date; /**< Date last accessed */
    uint16_t first_cluster_high; /**< First cluster (high word, only for FAT32) */
    uint16_t write_time;       /**< Time last modified */
    uint16_t write_date;       /**< Date last modified */
    uint16_t first_cluster_low; /**< First cluster (low word) */
    uint32_t file_size;        /**< File size (bytes) */
} FATDirEntry;

/**
 * Internal functions for FAT Driver
 */
// void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* buffer);
// int fat_driver_load_fat_table(FATDriver* driver);
// int fat_driver_load_root_directory(FATDriver* driver);
// int fat_driver_build_directory_tree(FATDriver* driver);
int fat_driver_build_directory_tree_recursive(FATDriver* driver, FileNode* directory);
uint32_t fat_driver_get_next_cluster(FATDriver* driver, uint32_t current_cluster);
uint32_t fat_driver_get_fat_entry(FATDriver* driver, uint32_t cluster);
void fat_driver_fill_file_node(FATDriver* driver, FileNode* node, const FATDirEntry* entry);

#endif // FAT_DRIVER_PRIVATE_H

