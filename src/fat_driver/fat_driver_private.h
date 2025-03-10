#ifndef FAT_DRIVER_PRIVATE_H
#define FAT_DRIVER_PRIVATE_H

#include "fat_driver_types.h"

/* Private Constants */
#define FAT_EOF_12 0xFFF
#define FAT_EOF_16 0xFFFF
#define FAT_EOF_32 0x0FFFFFFF

#define FAT_BAD_CLUSTER_12 0xFF7
#define FAT_BAD_CLUSTER_16 0xFFF7
#define FAT_BAD_CLUSTER_32 0x0FFFFFF7

#define FAT_FREE_CLUSTER 0x00000000

/* Private Types */
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

/* Private Function Prototypes */
int32_t fat_driver_read_boot_sector(fat_driver_private_t *driver);
int32_t fat_driver_read_fat_table(fat_driver_private_t *driver);
int32_t fat_driver_read_root_dir(fat_driver_private_t *driver);
int32_t fat_driver_get_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t *value);
int32_t fat_driver_set_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t value);
int32_t fat_driver_get_next_cluster(fat_driver_private_t *driver, uint32_t cluster, uint32_t *next_cluster);
int32_t fat_driver_is_eof_cluster(fat_driver_private_t *driver, uint32_t cluster);
int32_t fat_driver_is_bad_cluster(fat_driver_private_t *driver, uint32_t cluster);
int32_t fat_driver_is_free_cluster(uint32_t cluster);
int32_t fat_driver_get_cluster_offset(fat_driver_private_t *driver, uint32_t cluster);
int32_t fat_driver_get_root_dir_offset(fat_driver_private_t *driver, uint32_t entry_index);

/* Cache Functions */
int32_t fat_driver_cache_init(fat_driver_private_t *driver);
int32_t fat_driver_cache_read(fat_driver_private_t *driver, uint32_t sector, uint8_t *buffer);
int32_t fat_driver_cache_write(fat_driver_private_t *driver, uint32_t sector, const uint8_t *buffer);
int32_t fat_driver_cache_flush(fat_driver_private_t *driver);

/* Directory Functions */
int32_t fat_driver_find_dir_entry(fat_driver_private_t *driver, const char *path, fat_entry_t *entry);
int32_t fat_driver_create_dir_entry(fat_driver_private_t *driver, const char *path, fat_entry_t *entry);
int32_t fat_driver_delete_dir_entry(fat_driver_private_t *driver, const char *path);
int32_t fat_driver_update_dir_entry(fat_driver_private_t *driver, const char *path, fat_entry_t *entry);

/* File Functions */
int32_t fat_driver_create_file(fat_driver_private_t *driver, const char *path, uint8_t attributes);
int32_t fat_driver_delete_file(fat_driver_private_t *driver, const char *path);
int32_t fat_driver_rename_file(fat_driver_private_t *driver, const char *old_path, const char *new_path);
int32_t fat_driver_move_file(fat_driver_private_t *driver, const char *src_path, const char *dst_path);

#endif /* FAT_DRIVER_PRIVATE_H */ 