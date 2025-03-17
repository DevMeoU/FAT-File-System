#ifndef FAT_DRIVER_TYPES_H
#define FAT_DRIVER_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include "../common/common_types.h"

/* FAT Constants */
#define FAT_SECTOR_SIZE SECTOR_SIZE_512
#define FAT_CACHE_SIZE CACHE_SIZE_16
#define FAT_DIR_DELETED 0xE5

/* FAT Types */
#define FAT_TYPE_FAT12 FAT_TYPE_12
#define FAT_TYPE_FAT16 FAT_TYPE_16
#define FAT_TYPE_FAT32 FAT_TYPE_32

/* FAT EOC Values */
#define FAT_EOC_12 0x0FF8
#define FAT_EOC_16 0xFFF8
#define FAT_EOC_32 0x0FFFFFF8

/* FAT Bad Cluster Values */
#define FAT_BAD_CLUSTER_12 0x0FF7
#define FAT_BAD_CLUSTER_16 0xFFF7
#define FAT_BAD_CLUSTER_32 0x0FFFFFF7

/* FAT Free Cluster Value */
#define FAT_FREE_CLUSTER 0x0000

/* FAT Status Codes */
#define FAT_SUCCESS STATUS_SUCCESS
#define FAT_ERROR STATUS_ERROR
#define FAT_ERROR_INVALID STATUS_INVALID_PARAMETER
#define FAT_ERROR_NOT_FOUND STATUS_NOT_FOUND
#define FAT_ERROR_ACCESS_DENIED STATUS_ACCESS_DENIED
#define FAT_ERROR_ALREADY_EXISTS STATUS_ALREADY_EXISTS
#define FAT_ERROR_NOT_EMPTY STATUS_BUSY
#define FAT_ERROR_DISK_FULL STATUS_NO_MEMORY
#define FAT_ERROR_IO STATUS_IO_ERROR

/* FAT Boot Sector Structure */
typedef struct {
    uint8_t     jump_boot[3];
    uint8_t     oem_name[8];
    uint16_t    bytes_per_sector;
    uint8_t     sectors_per_cluster;
    uint16_t    reserved_sector_count;
    uint8_t     number_of_fats;
    uint16_t    root_entry_count;
    uint16_t    total_sectors_16;
    uint8_t     media;
    uint16_t    fat_size_16;
    uint16_t    sectors_per_track;
    uint16_t    number_of_heads;
    uint32_t    hidden_sectors;
    uint32_t    total_sectors_32;
    uint32_t    fat_size_32;
    uint16_t    ext_flags;
    uint16_t    fs_version;
    uint32_t    root_cluster;
    uint16_t    fs_info;
    uint16_t    backup_boot_sector;
    uint8_t     reserved[12];
    uint8_t     drive_number;
    uint8_t     reserved1;
    uint8_t     boot_signature;
    uint32_t    volume_id;
    uint8_t     volume_label[11];
    uint8_t     fs_type[8];
    uint8_t     boot_code[420];
    uint16_t    boot_sector_signature;
} __attribute__((packed)) fat_boot_sector_t;

/* FAT Path Context */
typedef struct {
    char current_path[APP_PATH_BUF_SIZE];
    char parent_path[APP_PATH_BUF_SIZE];
    uint32_t current_cluster;
    uint32_t parent_cluster;
    bool is_root;
} fat_path_context_t;

/* FAT Cache Entry Structure */
typedef struct {
    bool        valid;
    bool        dirty;
    uint32_t    sector;
    uint8_t     data[SECTOR_SIZE_512];
} fat_cache_entry_t;

/* FAT File Handle */
typedef struct {
    char name[APP_PATH_BUF_SIZE];
    uint32_t cluster;
    uint32_t position;
    uint32_t size;
    FatAccessMode mode;
    uint32_t current_sector;
    uint32_t sector_offset;
    uint8_t *sector_buffer;
    bool is_dirty;
} fat_file_t;

/* FAT Directory Handle */
typedef struct {
    uint32_t cluster;
    uint32_t current_sector;
    uint32_t sector_offset;
    uint8_t *sector_buffer;
} fat_dir_t;

/* FAT Directory Entry Structure */
typedef struct {
    uint8_t     name[11];
    uint8_t     attr;
    uint8_t     nt_res;
    uint8_t     create_time_tenth;
    uint16_t    create_time;
    uint16_t    create_date;
    uint16_t    access_date;
    uint16_t    first_cluster_hi;
    uint16_t    write_time;
    uint16_t    write_date;
    uint16_t    first_cluster_lo;
    uint32_t    file_size;
} __attribute__((packed)) fat_dir_entry_t;

/* FAT Driver Private Structure */
typedef struct {
    char img_path[APP_PATH_BUF_SIZE];
    uint32_t    mount_mode;
    FatType     fat_type;
    uint32_t    sectors_per_cluster;
    uint32_t    first_data_sector;
    uint32_t    root_dir_sectors;
    uint32_t    fat_table_size;
    uint8_t*    fat_table;
    uint32_t    root_dir_size;
    uint8_t*    root_dir;
    uint32_t    root_cluster;
    fat_config_t config;
    fat_boot_sector_t boot_sector;
    fat_cache_entry_t cache[CACHE_SIZE_16];
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