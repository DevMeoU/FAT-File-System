#ifndef FAT_DRIVER_TYPES_H
#define FAT_DRIVER_TYPES_H

#include <stdint.h>
#include "../common/common_types.h"

/* FAT Types */
typedef enum {
    FAT_TYPE_FAT12 = 12,
    FAT_TYPE_FAT16 = 16,
    FAT_TYPE_FAT32 = 32
} fat_type_t;

/* FAT File Attributes */
#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLUME_ID  0x08
#define FAT_ATTR_DIRECTORY  0x10
#define FAT_ATTR_ARCHIVE    0x20
#define FAT_ATTR_LONG_NAME  0x0F

/* FAT Constants */
#define FAT_SECTOR_SIZE 512
#define FAT_CACHE_SIZE 16
#define FAT_DIR_DELETED 0xE5

/* FAT File Modes */
#define FAT_MODE_READ 0x01
#define FAT_MODE_WRITE 0x02
#define FAT_MODE_APPEND 0x04
#define FAT_MODE_CREATE 0x08

/* FAT Mount Modes */
#define FAT_MOUNT_READ_ONLY 0x01
#define FAT_MOUNT_WRITE_PROTECT 0x02

/* FAT Error Codes */
#define FAT_ERROR_SUCCESS 0
#define FAT_ERROR_INVALID -1
#define FAT_ERROR_NOT_FOUND -2
#define FAT_ERROR_ACCESS_DENIED -3
#define FAT_ERROR_ALREADY_EXISTS -4
#define FAT_ERROR_NOT_EMPTY -5
#define FAT_ERROR_DISK_FULL -6
#define FAT_ERROR_IO -7

/* FAT Driver Configuration */
typedef struct {
    char img_path[256];
    uint32_t sector_size;
    uint32_t cluster_size;
    uint32_t fat_size;
    uint32_t root_dir_sectors;
    uint32_t first_data_sector;
    uint32_t total_clusters;
    fat_type_t fat_type;
} fat_config_t;

/* FAT File Handle */
typedef struct {
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector;
    uint32_t current_offset;
    uint32_t file_size;
    uint8_t attributes;
    uint8_t mode;
} fat_file_t;

/* FAT Directory Handle */
typedef struct {
    uint32_t first_cluster;
    uint32_t current_cluster;
    uint32_t current_sector;
    uint32_t current_entry;
    uint8_t attributes;
} fat_dir_t;

/* FAT Directory Entry */
typedef struct {
    char name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t last_access_date;
    uint16_t first_cluster_high;
    uint16_t write_time;
    uint16_t write_date;
    uint16_t first_cluster_low;
    uint32_t file_size;
} fat_dir_entry_t;

/* FAT Boot Sector */
typedef struct {
    uint8_t jump_boot[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t number_of_fats;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t number_of_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t ext_flags;
    uint16_t fs_version;
    uint32_t root_cluster;
    uint16_t fs_info;
    uint16_t backup_boot_sector;
    uint8_t reserved[12];
    uint8_t drive_number;
    uint8_t reserved1;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} fat_boot_sector_t;

/* FAT Path Context */
typedef struct {
    char current_path[1024];
    char parent_path[1024];
    uint32_t current_cluster;
    uint32_t parent_cluster;
    bool is_root;
} fat_path_context_t;

/* FAT Cache Entry */
typedef struct {
    bool valid;
    bool dirty;
    uint32_t sector;
    uint8_t data[FAT_SECTOR_SIZE];
} fat_cache_entry_t;

/* FAT Driver Private */
typedef struct {
    fat_config_t config;
    fat_boot_sector_t boot_sector;
    uint8_t *fat_table;
    uint32_t fat_table_size;
    uint8_t *root_dir;
    uint32_t root_dir_size;
    uint32_t root_cluster;
    uint32_t mount_mode;
    fat_cache_entry_t cache[FAT_CACHE_SIZE];
} fat_driver_private_t;

/* FAT Driver */
typedef struct {
    fat_driver_private_t private;
    fat_file_t* current_file;
    fat_dir_t* current_dir;
} FATDriver;

#endif /* FAT_DRIVER_TYPES_H */

/*********************************************************************
 * UUID: 4f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/ 