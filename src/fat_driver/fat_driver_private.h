#ifndef FAT_DRIVER_PRIVATE_H
#define FAT_DRIVER_PRIVATE_H

#include "fat_driver_types.h"

/* FAT Driver Private Functions */
static int32_t fat_driver_init(fat_driver_private_t *driver);
static int32_t fat_driver_init_storage(void);
static int32_t fat_driver_validate_boot_sector(void);
static int32_t fat_driver_init_fat_tables(void);
static uint32_t fat_driver_get_fat_size(const fat_boot_sector_t *boot_sector);
static int32_t fat_driver_calculate_layout(const fat_boot_sector_t *boot_sector);
static int32_t fat_driver_is_directory(const fat_dir_entry_t *entry);
static int32_t fat_driver_list_directory(const char *path);
static int32_t fat_driver_change_directory(const char *path);
static int32_t fat_driver_resolve_path(const char *path, char *resolved_path);
static int32_t fat_driver_normalize_path(const char *path, char *normalized_path);
static int32_t fat_driver_get_parent_path(const char *path, char *parent_path);
static int32_t fat_driver_init_path_context(void);
static int32_t fat_driver_update_path_context(const char *new_path, uint32_t new_cluster);

/* Cluster Management Functions */
static int32_t fat_driver_get_next_cluster(fat_driver_private_t *driver, uint32_t cluster, uint32_t *next_cluster);
static int32_t fat_driver_is_eof_cluster(fat_driver_private_t *driver, uint32_t cluster);
static int32_t fat_driver_cluster_to_sector(fat_driver_private_t *driver, uint32_t cluster);
static int32_t fat_driver_alloc_cluster(fat_driver_private_t *driver, uint32_t *cluster);
static int32_t fat_driver_free_cluster(fat_driver_private_t *driver, uint32_t cluster);
static int32_t fat_driver_is_free_cluster(uint32_t cluster);

/* Sector Management Functions */
static int32_t fat_driver_read_sector(fat_driver_private_t *driver, uint32_t sector, uint8_t *buffer);

/* Directory Functions */
static int32_t fat_driver_find_dir_entry(fat_driver_private_t *driver, const char *path, fat_dir_entry_t *entry);
static int32_t fat_driver_convert_to_short_name(const char *path, char *short_name);

/* Time Functions */
static uint16_t fat_driver_get_time(void);
static uint16_t fat_driver_get_date(void);

#endif /* FAT_DRIVER_PRIVATE_H */ 