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
#include "fat_driver_types.h"
#include "../common/common_types.h"

/* Private constants */
#define FAT_SECTOR_SIZE        512
#define FAT_DIR_ENTRY_SIZE     32
#define FAT_DIR_NAME_LEN       8
#define FAT_DIR_EXT_LEN        3
#define FAT_CACHE_SIZE         4

#define FAT_DIR_EMPTY         0x00
#define FAT_DIR_DELETED       0xE5

/* FAT32 EOC và Mask */
#ifndef FAT32_EOC
#define FAT32_EOC     0x0FFFFFF8
#endif

#ifndef FAT32_MASK
#define FAT32_MASK    0x0FFFFFFF
#endif

/* Các macro hỗ trợ */
#define FAT_EOC(type) ((type) == FAT_TYPE_12 ? FAT12_EOC : \
                     ((type) == FAT_TYPE_16 ? FAT16_EOC : \
                      FAT32_EOC))

#define FAT_MASK(type) ((type) == FAT_TYPE_12 ? FAT12_MASK : \
                       ((type) == FAT_TYPE_16 ? FAT16_MASK : \
                        FAT32_MASK))

/* Private data structures */
typedef struct {
    uint32_t sector;
    uint8_t data[FAT_SECTOR_SIZE];
    bool valid;
    bool dirty;
} fat_cache_entry_t;

typedef struct {
    uint8_t  fat_type;       /* Loại FAT (12/16/32) */
    uint32_t sectors_per_cluster;
    uint32_t reserved_sectors;
    uint32_t root_dir_sectors;
    uint32_t first_data_sector;
    uint32_t total_clusters;
} fat_config_internal_t;

typedef struct {
    fat_config_internal_t config;
    uint32_t fat_start;
    uint32_t fat_size;          /* Kích thước FAT theo sector */
    uint32_t root_cluster;      /* Cluster đầu tiên của thư mục gốc (FAT32) */
    uint32_t first_fat_sector;  /* Sector đầu tiên của bảng FAT */
    bool mounted;               /* Đã mount hay chưa */
    
    /* Cache cho sectors */
    fat_cache_entry_t cache[FAT_CACHE_SIZE];
} fat_context_t;

/* Private function declarations */
static uint8_t fat_calculate_short_name_checksum(const char *short_name);
static void fat_get_name(const fat_dir_entry_t *entry, char *name);
static uint32_t fat_cluster_to_sector(uint32_t cluster);
static uint16_t fat_get_time(void);
static uint16_t fat_get_date(void);
static void calculate_layout(const fat_boot_sector_t *boot_sector);
static uint32_t get_next_cluster(uint32_t current_cluster);
static int32_t read_sector(uint32_t sector, uint8_t *buffer);
static int32_t write_sector(uint32_t sector, const uint8_t *buffer);

/* Global variables */
extern fat_context_t fat_ctx;
extern fat_boot_sector_t boot_sector;
extern uint32_t fat_type;

/* Public function declarations */
int32_t fat_find_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_create_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_write_dir_entry(const fat_dir_entry_t *entry);
uint32_t fat_alloc_cluster(void);
int32_t fat_free_cluster(uint32_t cluster);

#endif /* FAT_DRIVER_PRIVATE_H */

/*********************************************************************
 * UUID: 1c9d2f1b-4c4a-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 