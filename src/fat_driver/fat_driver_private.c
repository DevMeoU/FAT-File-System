#define _GNU_SOURCE  /* For strdup */
/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm private của module FAT Driver.
 *   Các hàm này chỉ được sử dụng nội bộ trong module.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "../common/common_types.h"
#include "fat_driver_types.h"
#include "fat_driver_private.h"
#include "../hal/hal.h"
#include "../utilities/log/print_color.h"

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

int32_t fat_driver_read_boot_sector(fat_driver_private_t *driver)
{
    uint8_t buffer[FAT_SECTOR_SIZE];
    int32_t status = hal_read_sector(0, buffer);
    if (status != FAT_SUCCESS) {
        return status;
    }

    memcpy(&driver->boot_sector, buffer, sizeof(fat_boot_sector_t));

    if (driver->boot_sector.boot_signature != 0xAA55) {
        return FAT_ERROR_INVALID;
    }

    return FAT_SUCCESS;
}

int32_t fat_driver_read_fat_table(fat_driver_private_t *driver)
{
    uint32_t fat_size = driver->boot_sector.fat_size_16 ? 
                        driver->boot_sector.fat_size_16 : 
                        driver->boot_sector.fat_size_32;

    driver->fat_table_size = fat_size * FAT_SECTOR_SIZE;
    driver->fat_table = malloc(driver->fat_table_size);
    if (!driver->fat_table) {
        return FAT_ERROR_IO;
    }

    for (uint32_t i = 0; i < fat_size; i++) {
        int32_t status = hal_read_sector(driver->boot_sector.reserved_sector_count + i,
            driver->fat_table + (i * FAT_SECTOR_SIZE));
        if (status != FAT_SUCCESS) {
            free(driver->fat_table);
            driver->fat_table = NULL;
            return status;
        }
    }

    return FAT_SUCCESS;
}

int32_t fat_driver_read_root_dir(fat_driver_private_t *driver)
{
    uint32_t root_dir_size = driver->boot_sector.root_entry_count * sizeof(fat_dir_entry_t);
    driver->root_dir_size = root_dir_size;
    driver->root_dir = malloc(root_dir_size);
    if (!driver->root_dir) {
        return FAT_ERROR_IO;
    }

    uint32_t root_dir_sectors = (root_dir_size + FAT_SECTOR_SIZE - 1) / FAT_SECTOR_SIZE;
    uint32_t root_dir_sector = driver->boot_sector.reserved_sector_count +
                              driver->boot_sector.fat_size_16 * driver->boot_sector.number_of_fats;

    for (uint32_t i = 0; i < root_dir_sectors; i++) {
        int32_t status = hal_read_sector(root_dir_sector + i,
            driver->root_dir + (i * FAT_SECTOR_SIZE));
        if (status != FAT_SUCCESS) {
            free(driver->root_dir);
            driver->root_dir = NULL;
            return status;
        }
    }

    return FAT_SUCCESS;
}

int32_t fat_driver_get_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t *value)
{
    uint32_t offset;
    uint32_t mask;

    switch (driver->fat_type) {
        case FAT_TYPE_FAT12:
            offset = cluster + (cluster / 2);
            *value = *(uint16_t *)(driver->fat_table + offset);
            if (cluster & 1) {
                *value >>= 4;
            }
            *value &= 0x0FFF;
            break;

        case FAT_TYPE_FAT16:
            offset = cluster * 2;
            *value = *(uint16_t *)(driver->fat_table + offset);
            break;

        case FAT_TYPE_FAT32:
            offset = cluster * 4;
            *value = *(uint32_t *)(driver->fat_table + offset) & 0x0FFFFFFF;
            break;

        default:
            return FAT_ERROR_INVALID;
    }

    return FAT_SUCCESS;
}

int32_t fat_driver_set_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t value)
{
    uint32_t offset;
    uint32_t mask;

    switch (driver->fat_type) {
        case FAT_TYPE_FAT12:
            offset = cluster + (cluster / 2);
            if (cluster & 1) {
                *(uint16_t *)(driver->fat_table + offset) &= 0x000F;
                *(uint16_t *)(driver->fat_table + offset) |= (value << 4);
            } else {
                *(uint16_t *)(driver->fat_table + offset) &= 0xF000;
                *(uint16_t *)(driver->fat_table + offset) |= value;
            }
            break;

        case FAT_TYPE_FAT16:
            offset = cluster * 2;
            *(uint16_t *)(driver->fat_table + offset) = value;
            break;

        case FAT_TYPE_FAT32:
            offset = cluster * 4;
            *(uint32_t *)(driver->fat_table + offset) &= 0xF0000000;
            *(uint32_t *)(driver->fat_table + offset) |= value;
            break;

        default:
            return FAT_ERROR_INVALID;
    }

    return FAT_SUCCESS;
}

int32_t fat_driver_get_next_cluster(fat_driver_private_t *driver, uint32_t cluster, uint32_t *next_cluster)
{
    return fat_driver_get_cluster_value(driver, cluster, next_cluster);
}

int32_t fat_driver_is_eof_cluster(fat_driver_private_t *driver, uint32_t cluster)
{
    uint32_t value;
    int32_t status = fat_driver_get_cluster_value(driver, cluster, &value);
    if (status != FAT_SUCCESS) {
        return status;
    }

    switch (driver->fat_type) {
        case FAT_TYPE_FAT12:
            return (value >= FAT_EOC_12);
        case FAT_TYPE_FAT16:
            return (value >= FAT_EOC_16);
        case FAT_TYPE_FAT32:
            return (value >= FAT_EOC_32);
        default:
            return FAT_ERROR_INVALID;
    }
}

int32_t fat_driver_is_bad_cluster(fat_driver_private_t *driver, uint32_t cluster)
{
    uint32_t value;
    int32_t status = fat_driver_get_cluster_value(driver, cluster, &value);
    if (status != FAT_SUCCESS) {
        return status;
    }

    switch (driver->fat_type) {
        case FAT_TYPE_FAT12:
            return (value == FAT_BAD_CLUSTER_12);
        case FAT_TYPE_FAT16:
            return (value == FAT_BAD_CLUSTER_16);
        case FAT_TYPE_FAT32:
            return (value == FAT_BAD_CLUSTER_32);
        default:
            return FAT_ERROR_INVALID;
    }
}

int32_t fat_driver_is_free_cluster(uint32_t cluster)
{
    return (cluster == FAT_FREE_CLUSTER) ? FAT_SUCCESS : FAT_ERROR_INVALID;
}

int32_t fat_driver_get_cluster_offset(fat_driver_private_t *driver, uint32_t cluster)
{
    return ((cluster - 2) * driver->sectors_per_cluster) + driver->first_data_sector;
}

int32_t fat_driver_cache_init(fat_driver_private_t *driver)
{
    memset(driver->cache, 0, sizeof(driver->cache));
    return FAT_SUCCESS;
}

int32_t fat_driver_cache_read(fat_driver_private_t *driver, uint32_t sector, uint8_t *buffer)
{
    /* Check cache first */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (driver->cache[i].valid && driver->cache[i].sector == sector) {
            memcpy(buffer, driver->cache[i].data, FAT_SECTOR_SIZE);
            return FAT_SUCCESS;
        }
    }

    /* Not in cache, read from disk */
    int32_t status = hal_read_sector(sector, buffer);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Add to cache */
    int cache_index = 0;
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (!driver->cache[i].valid) {
            cache_index = i;
            break;
        }
    }

    driver->cache[cache_index].valid = true;
    driver->cache[cache_index].dirty = false;
    driver->cache[cache_index].sector = sector;
    memcpy(driver->cache[cache_index].data, buffer, FAT_SECTOR_SIZE);

    return FAT_SUCCESS;
}

int32_t fat_driver_cache_write(fat_driver_private_t *driver, uint32_t sector, const uint8_t *buffer)
{
    /* Check cache first */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (driver->cache[i].valid && driver->cache[i].sector == sector) {
            memcpy(driver->cache[i].data, buffer, FAT_SECTOR_SIZE);
            driver->cache[i].dirty = true;
            return FAT_SUCCESS;
        }
    }

    /* Not in cache, add it */
    int cache_index = 0;
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (!driver->cache[i].valid) {
            cache_index = i;
            break;
        }
    }

    driver->cache[cache_index].valid = true;
    driver->cache[cache_index].dirty = true;
    driver->cache[cache_index].sector = sector;
    memcpy(driver->cache[cache_index].data, buffer, FAT_SECTOR_SIZE);

    return FAT_SUCCESS;
}

int32_t fat_driver_cache_flush(fat_driver_private_t *driver)
{
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (driver->cache[i].valid && driver->cache[i].dirty) {
            int32_t status = hal_write_sector(driver->cache[i].sector, driver->cache[i].data);
            if (status != FAT_SUCCESS) {
                return status;
            }
            driver->cache[i].dirty = false;
        }
    }

    return FAT_SUCCESS;
}

int32_t fat_driver_find_dir_entry(fat_driver_private_t *driver, const char *path, fat_dir_entry_t *entry)
{
    fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)driver->root_dir;

    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0) {
            break;
        }

        if (dir_entry[i].name[0] == FAT_DIR_DELETED) {
            continue;
        }

        char short_name[12];
        int32_t status = fat_driver_convert_to_short_name(path, short_name);
        if (status != FAT_SUCCESS) {
            return status;
        }

        if (memcmp(dir_entry[i].name, short_name, 11) == 0) {
            memcpy(entry, &dir_entry[i], sizeof(fat_dir_entry_t));
            return FAT_SUCCESS;
        }
    }

    return FAT_ERROR_NOT_FOUND;
}

int32_t fat_driver_create_dir_entry(fat_driver_private_t *driver, const char *path, fat_dir_entry_t *entry)
{
    fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)driver->root_dir;

    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0 || dir_entry[i].name[0] == FAT_DIR_DELETED) {
            memcpy(&dir_entry[i], entry, sizeof(fat_dir_entry_t));
            return FAT_SUCCESS;
        }
    }

    return FAT_ERROR_DISK_FULL;
}

int32_t fat_driver_delete_dir_entry(fat_driver_private_t *driver, const char *path)
{
    fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)driver->root_dir;

    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0) {
            break;
        }

        if (dir_entry[i].name[0] == FAT_DIR_DELETED) {
            continue;
        }

        char short_name[12];
        int32_t status = fat_driver_convert_to_short_name(path, short_name);
        if (status != FAT_SUCCESS) {
            return status;
        }

        if (memcmp(dir_entry[i].name, short_name, 11) == 0) {
            dir_entry[i].name[0] = FAT_DIR_DELETED;
            return FAT_SUCCESS;
        }
    }

    return FAT_ERROR_NOT_FOUND;
}

int32_t fat_driver_update_dir_entry(fat_driver_private_t *driver, const char *path, fat_dir_entry_t *entry)
{
    fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)driver->root_dir;

    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0) {
            break;
        }

        if (dir_entry[i].name[0] == FAT_DIR_DELETED) {
            continue;
        }

        char short_name[12];
        int32_t status = fat_driver_convert_to_short_name(path, short_name);
        if (status != FAT_SUCCESS) {
            return status;
        }

        if (memcmp(dir_entry[i].name, short_name, 11) == 0) {
            memcpy(&dir_entry[i], entry, sizeof(fat_dir_entry_t));
            return FAT_SUCCESS;
        }
    }

    return FAT_ERROR_NOT_FOUND;
}

int32_t fat_driver_create_file(fat_driver_private_t *driver, const char *path, fat_dir_entry_t *entry)
{
    /* Convert path to short name */
    char short_name[12];
    int32_t status = fat_driver_convert_to_short_name(path, short_name);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Initialize directory entry */
    memset(entry, 0, sizeof(fat_dir_entry_t));
    memcpy(entry->name, short_name, 11);
    entry->attr = 0;
    entry->create_time = fat_driver_get_time();
    entry->create_date = fat_driver_get_date();
    entry->write_time = entry->create_time;
    entry->write_date = entry->create_date;
    entry->access_date = entry->create_date;

    /* Allocate first cluster */
    uint32_t cluster;
    status = fat_driver_alloc_cluster(driver, &cluster);
    if (status != FAT_SUCCESS) {
        return status;
    }

    entry->first_cluster_hi = (uint16_t)(cluster >> 16);
    entry->first_cluster_lo = (uint16_t)cluster;
    entry->file_size = 0;

    /* Create directory entry */
    return fat_driver_create_dir_entry(driver, path, entry);
}

int32_t fat_driver_delete_file(fat_driver_private_t *driver, const char *path)
{
    fat_dir_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(driver, path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Free clusters */
    uint32_t cluster = ((uint32_t)entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    while (!fat_driver_is_eof_cluster(driver, cluster)) {
        uint32_t next_cluster;
        status = fat_driver_get_next_cluster(driver, cluster, &next_cluster);
        if (status != FAT_SUCCESS) {
            return status;
        }

        status = fat_driver_free_cluster(driver, cluster);
        if (status != FAT_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    /* Delete directory entry */
    return fat_driver_delete_dir_entry(driver, path);
}

int32_t fat_driver_rename_file(fat_driver_private_t *driver, const char *old_path, const char *new_path)
{
    fat_dir_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(driver, old_path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Convert new path to short name */
    char short_name[12];
    status = fat_driver_convert_to_short_name(new_path, short_name);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Update directory entry */
    memcpy(entry.name, short_name, 11);
    return fat_driver_update_dir_entry(driver, old_path, &entry);
}

int32_t fat_driver_move_file(fat_driver_private_t *driver, const char *src_path, const char *dst_path)
{
    fat_dir_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(driver, src_path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Convert destination path to short name */
    char short_name[12];
    status = fat_driver_convert_to_short_name(dst_path, short_name);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Create new directory entry */
    memcpy(entry.name, short_name, 11);
    status = fat_driver_create_dir_entry(driver, dst_path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Delete old directory entry */
    return fat_driver_delete_dir_entry(driver, src_path);
}

int32_t fat_driver_convert_to_short_name(const char *path, char *short_name)
{
    const char *base = strrchr(path, '/');
    if (base) {
        base++;
    } else {
        base = path;
    }

    /* Convert to uppercase and pad with spaces */
    memset(short_name, ' ', 11);
    for (int i = 0; i < 8 && base[i] && base[i] != '.'; i++) {
        short_name[i] = toupper(base[i]);
    }

    /* Copy extension */
    const char *ext = strchr(base, '.');
    if (ext) {
        ext++;
        for (int i = 0; i < 3 && ext[i]; i++) {
            short_name[8 + i] = toupper(ext[i]);
        }
    }

    return FAT_SUCCESS;
}

int32_t fat_driver_alloc_cluster(fat_driver_private_t *driver, uint32_t *cluster)
{
    /* Find free cluster */
    for (uint32_t i = 2; i < driver->fat_table_size / 2; i++) {
        uint32_t value;
        int32_t status = fat_driver_get_cluster_value(driver, i, &value);
        if (status != FAT_SUCCESS) {
            return status;
        }

        if (value == FAT_FREE_CLUSTER) {
            /* Mark cluster as end of chain */
            switch (driver->fat_type) {
                case FAT_TYPE_FAT12:
                    status = fat_driver_set_cluster_value(driver, i, FAT_EOC_12);
                    break;
                case FAT_TYPE_FAT16:
                    status = fat_driver_set_cluster_value(driver, i, FAT_EOC_16);
                    break;
                case FAT_TYPE_FAT32:
                    status = fat_driver_set_cluster_value(driver, i, FAT_EOC_32);
                    break;
                default:
                    return FAT_ERROR_INVALID;
            }

            if (status != FAT_SUCCESS) {
                return status;
            }

            *cluster = i;
            return FAT_SUCCESS;
        }
    }

    return FAT_ERROR_DISK_FULL;
}

int32_t fat_driver_free_cluster(fat_driver_private_t *driver, uint32_t cluster)
{
    /* Mark cluster as free */
    return fat_driver_set_cluster_value(driver, cluster, FAT_FREE_CLUSTER);
}

uint16_t fat_driver_get_time(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ((tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2));
}

uint16_t fat_driver_get_date(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return (((tm->tm_year - 80) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday);
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 