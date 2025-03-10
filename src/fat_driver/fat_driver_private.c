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
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    /* Read boot sector */
    uint8_t buffer[FAT_SECTOR_SIZE];
    int32_t status = hal_read_sector(0, buffer);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Parse boot sector */
    memcpy(&driver->boot_sector, buffer, sizeof(fat_boot_sector_t));

    /* Validate boot sector */
    if (driver->boot_sector.boot_signature != 0xAA55) {
        return FAT_ERROR_INVALID;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_read_fat_table(fat_driver_private_t *driver)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    /* Calculate FAT size */
    uint32_t fat_size = fat_driver_get_fat_size(&driver->boot_sector);
    if (fat_size == 0) {
        return FAT_ERROR_INVALID;
    }

    /* Allocate memory for FAT table */
    driver->fat_table = (uint8_t *)malloc(fat_size);
    if (!driver->fat_table) {
        return FAT_ERROR_IO;
    }

    /* Read FAT table */
    for (uint32_t i = 0; i < fat_size / FAT_SECTOR_SIZE; i++) {
        int32_t status = hal_read_sector(driver->boot_sector.reserved_sector_count + i,
                                        &driver->fat_table[i * FAT_SECTOR_SIZE]);
        if (status != FAT_ERROR_SUCCESS) {
            free(driver->fat_table);
            driver->fat_table = NULL;
            return status;
        }
    }

    driver->fat_table_size = fat_size;
    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_read_root_dir(fat_driver_private_t *driver)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    /* Calculate root directory size */
    uint32_t root_dir_size = driver->boot_sector.root_entry_count * sizeof(fat_entry_t);
    if (root_dir_size == 0) {
        return FAT_ERROR_INVALID;
    }

    /* Allocate memory for root directory */
    driver->root_dir = (uint8_t *)malloc(root_dir_size);
    if (!driver->root_dir) {
        return FAT_ERROR_IO;
    }

    /* Read root directory */
    uint32_t root_dir_sector = driver->boot_sector.reserved_sector_count +
                              driver->boot_sector.fat_size_16 * driver->boot_sector.number_of_fats;
    for (uint32_t i = 0; i < root_dir_size / FAT_SECTOR_SIZE; i++) {
        int32_t status = hal_read_sector(root_dir_sector + i,
                                        &driver->root_dir[i * FAT_SECTOR_SIZE]);
        if (status != FAT_ERROR_SUCCESS) {
            free(driver->root_dir);
            driver->root_dir = NULL;
            return status;
        }
    }

    driver->root_dir_size = root_dir_size;
    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_get_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t *value)
{
    if (!driver || !value) {
        return FAT_ERROR_INVALID;
    }

    /* Calculate FAT offset */
    uint32_t fat_offset;
    switch (driver->config.fat_type) {
        case FAT_TYPE_FAT12:
            fat_offset = cluster * 3 / 2;
            break;
        case FAT_TYPE_FAT16:
            fat_offset = cluster * 2;
            break;
        case FAT_TYPE_FAT32:
            fat_offset = cluster * 4;
            break;
        default:
            return FAT_ERROR_INVALID;
    }

    /* Read FAT value */
    switch (driver->config.fat_type) {
        case FAT_TYPE_FAT12: {
            uint16_t fat_value = *(uint16_t *)&driver->fat_table[fat_offset];
            if (cluster & 1) {
                *value = fat_value >> 4;
            } else {
                *value = fat_value & 0xFFF;
            }
            break;
        }
        case FAT_TYPE_FAT16:
            *value = *(uint16_t *)&driver->fat_table[fat_offset];
            break;
        case FAT_TYPE_FAT32:
            *value = *(uint32_t *)&driver->fat_table[fat_offset] & 0x0FFFFFFF;
            break;
        default:
            return FAT_ERROR_INVALID;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_set_cluster_value(fat_driver_private_t *driver, uint32_t cluster, uint32_t value)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    /* Calculate FAT offset */
    uint32_t fat_offset;
    switch (driver->config.fat_type) {
        case FAT_TYPE_FAT12:
            fat_offset = cluster * 3 / 2;
            break;
        case FAT_TYPE_FAT16:
            fat_offset = cluster * 2;
            break;
        case FAT_TYPE_FAT32:
            fat_offset = cluster * 4;
            break;
        default:
            return FAT_ERROR_INVALID;
    }

    /* Write FAT value */
    switch (driver->config.fat_type) {
        case FAT_TYPE_FAT12: {
            uint16_t *fat_value = (uint16_t *)&driver->fat_table[fat_offset];
            if (cluster & 1) {
                *fat_value = (*fat_value & 0x0FFF) | (value << 4);
            } else {
                *fat_value = (*fat_value & 0xF000) | (value & 0xFFF);
            }
            break;
        }
        case FAT_TYPE_FAT16:
            *(uint16_t *)&driver->fat_table[fat_offset] = (uint16_t)value;
            break;
        case FAT_TYPE_FAT32:
            *(uint32_t *)&driver->fat_table[fat_offset] = value;
            break;
        default:
            return FAT_ERROR_INVALID;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_get_next_cluster(fat_driver_private_t *driver, uint32_t cluster, uint32_t *next_cluster)
{
    if (!driver || !next_cluster) {
        return FAT_ERROR_INVALID;
    }

    return fat_driver_get_cluster_value(driver, cluster, next_cluster);
}

int32_t fat_driver_is_eof_cluster(fat_driver_private_t *driver, uint32_t cluster)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    uint32_t value;
    int32_t status = fat_driver_get_cluster_value(driver, cluster, &value);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    switch (driver->config.fat_type) {
        case FAT_TYPE_FAT12:
            return (value >= 0xFF8) ? FAT_ERROR_SUCCESS : FAT_ERROR_INVALID;
        case FAT_TYPE_FAT16:
            return (value >= 0xFFF8) ? FAT_ERROR_SUCCESS : FAT_ERROR_INVALID;
        case FAT_TYPE_FAT32:
            return (value >= 0x0FFFFFF8) ? FAT_ERROR_SUCCESS : FAT_ERROR_INVALID;
        default:
            return FAT_ERROR_INVALID;
    }
}

int32_t fat_driver_is_bad_cluster(fat_driver_private_t *driver, uint32_t cluster)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    uint32_t value;
    int32_t status = fat_driver_get_cluster_value(driver, cluster, &value);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    switch (driver->config.fat_type) {
        case FAT_TYPE_FAT12:
            return (value == 0xFF7) ? FAT_ERROR_SUCCESS : FAT_ERROR_INVALID;
        case FAT_TYPE_FAT16:
            return (value == 0xFFF7) ? FAT_ERROR_SUCCESS : FAT_ERROR_INVALID;
        case FAT_TYPE_FAT32:
            return (value == 0x0FFFFFF7) ? FAT_ERROR_SUCCESS : FAT_ERROR_INVALID;
        default:
            return FAT_ERROR_INVALID;
    }
}

int32_t fat_driver_is_free_cluster(uint32_t cluster)
{
    return (cluster == 0) ? FAT_ERROR_SUCCESS : FAT_ERROR_INVALID;
}

int32_t fat_driver_get_cluster_offset(fat_driver_private_t *driver, uint32_t cluster)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    return ((cluster - 2) * driver->config.sectors_per_cluster) + driver->config.first_data_sector;
}

int32_t fat_driver_get_root_dir_offset(fat_driver_private_t *driver, uint32_t entry_index)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    return entry_index * sizeof(fat_entry_t);
}

int32_t fat_driver_cache_init(fat_driver_private_t *driver)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    memset(driver->cache, 0, sizeof(driver->cache));
    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_cache_read(fat_driver_private_t *driver, uint32_t sector, uint8_t *buffer)
{
    if (!driver || !buffer) {
        return FAT_ERROR_INVALID;
    }

    /* Check cache */
    for (uint32_t i = 0; i < FAT_CACHE_SIZE; i++) {
        if (driver->cache[i].valid && driver->cache[i].sector == sector) {
            memcpy(buffer, driver->cache[i].data, FAT_SECTOR_SIZE);
            return FAT_ERROR_SUCCESS;
        }
    }

    /* Read from disk */
    int32_t status = hal_read_sector(sector, buffer);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Update cache */
    for (uint32_t i = 0; i < FAT_CACHE_SIZE; i++) {
        if (!driver->cache[i].valid) {
            driver->cache[i].valid = true;
            driver->cache[i].dirty = false;
            driver->cache[i].sector = sector;
            memcpy(driver->cache[i].data, buffer, FAT_SECTOR_SIZE);
            return FAT_ERROR_SUCCESS;
        }
    }

    /* Cache is full, write back first entry */
    if (driver->cache[0].dirty) {
        status = hal_write_sector(driver->cache[0].sector, driver->cache[0].data);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }
    }

    /* Shift cache entries */
    for (uint32_t i = 0; i < FAT_CACHE_SIZE - 1; i++) {
        driver->cache[i] = driver->cache[i + 1];
    }

    /* Add new entry */
    driver->cache[FAT_CACHE_SIZE - 1].valid = true;
    driver->cache[FAT_CACHE_SIZE - 1].dirty = false;
    driver->cache[FAT_CACHE_SIZE - 1].sector = sector;
    memcpy(driver->cache[FAT_CACHE_SIZE - 1].data, buffer, FAT_SECTOR_SIZE);

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_cache_write(fat_driver_private_t *driver, uint32_t sector, const uint8_t *buffer)
{
    if (!driver || !buffer) {
        return FAT_ERROR_INVALID;
    }

    /* Check cache */
    for (uint32_t i = 0; i < FAT_CACHE_SIZE; i++) {
        if (driver->cache[i].valid && driver->cache[i].sector == sector) {
            memcpy(driver->cache[i].data, buffer, FAT_SECTOR_SIZE);
            driver->cache[i].dirty = true;
            return FAT_ERROR_SUCCESS;
        }
    }

    /* Write to disk */
    return hal_write_sector(sector, buffer);
}

int32_t fat_driver_cache_flush(fat_driver_private_t *driver)
{
    if (!driver) {
        return FAT_ERROR_INVALID;
    }

    /* Write back all dirty entries */
    for (uint32_t i = 0; i < FAT_CACHE_SIZE; i++) {
        if (driver->cache[i].valid && driver->cache[i].dirty) {
            int32_t status = hal_write_sector(driver->cache[i].sector, driver->cache[i].data);
            if (status != FAT_ERROR_SUCCESS) {
                return status;
            }
            driver->cache[i].dirty = false;
        }
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_find_dir_entry(fat_driver_private_t *driver, const char *path, fat_entry_t *entry)
{
    if (!driver || !path || !entry) {
        return FAT_ERROR_INVALID;
    }

    /* Find file entry */
    fat_entry_t *dir_entry = (fat_entry_t *)driver->root_dir;
    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0) {
            /* End of directory */
            break;
        }

        if (dir_entry[i].name[0] == FAT_DIR_DELETED) {
            continue;
        }

        /* Compare names */
        char name[11];
        strncpy(name, dir_entry[i].name, 11);
        name[11] = '\0';

        if (strcmp(name, path) == 0) {
            memcpy(entry, &dir_entry[i], sizeof(fat_entry_t));
            return FAT_ERROR_SUCCESS;
        }
    }

    return FAT_ERROR_NOT_FOUND;
}

int32_t fat_driver_create_dir_entry(fat_driver_private_t *driver, const char *path, fat_entry_t *entry)
{
    if (!driver || !path || !entry) {
        return FAT_ERROR_INVALID;
    }

    /* Find free directory entry */
    fat_entry_t *dir_entry = (fat_entry_t *)driver->root_dir;
    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0 || dir_entry[i].name[0] == FAT_DIR_DELETED) {
            /* Copy entry */
            memcpy(&dir_entry[i], entry, sizeof(fat_entry_t));
            return FAT_ERROR_SUCCESS;
        }
    }

    return FAT_ERROR_DISK_FULL;
}

int32_t fat_driver_delete_dir_entry(fat_driver_private_t *driver, const char *path)
{
    if (!driver || !path) {
        return FAT_ERROR_INVALID;
    }

    /* Find directory entry */
    fat_entry_t *dir_entry = (fat_entry_t *)driver->root_dir;
    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0) {
            /* End of directory */
            break;
        }

        if (dir_entry[i].name[0] == FAT_DIR_DELETED) {
            continue;
        }

        /* Compare names */
        char name[11];
        strncpy(name, dir_entry[i].name, 11);
        name[11] = '\0';

        if (strcmp(name, path) == 0) {
            /* Mark as deleted */
            dir_entry[i].name[0] = FAT_DIR_DELETED;
            return FAT_ERROR_SUCCESS;
        }
    }

    return FAT_ERROR_NOT_FOUND;
}

int32_t fat_driver_update_dir_entry(fat_driver_private_t *driver, const char *path, fat_entry_t *entry)
{
    if (!driver || !path || !entry) {
        return FAT_ERROR_INVALID;
    }

    /* Find directory entry */
    fat_entry_t *dir_entry = (fat_entry_t *)driver->root_dir;
    for (uint32_t i = 0; i < driver->boot_sector.root_entry_count; i++) {
        if (dir_entry[i].name[0] == 0) {
            /* End of directory */
            break;
        }

        if (dir_entry[i].name[0] == FAT_DIR_DELETED) {
            continue;
        }

        /* Compare names */
        char name[11];
        strncpy(name, dir_entry[i].name, 11);
        name[11] = '\0';

        if (strcmp(name, path) == 0) {
            /* Update entry */
            memcpy(&dir_entry[i], entry, sizeof(fat_entry_t));
            return FAT_ERROR_SUCCESS;
        }
    }

    return FAT_ERROR_NOT_FOUND;
}

int32_t fat_driver_create_file(fat_driver_private_t *driver, const char *path, uint8_t attributes)
{
    if (!driver || !path) {
        return FAT_ERROR_INVALID;
    }

    /* Create directory entry */
    fat_entry_t entry;
    memset(&entry, 0, sizeof(entry));
    entry.attributes = attributes;
    entry.first_cluster_high = 0;
    entry.first_cluster_low = 0;
    entry.file_size = 0;
    entry.write_date = fat_driver_get_date();
    entry.write_time = fat_driver_get_time();

    /* Convert name */
    char short_name[11];
    int32_t status = fat_driver_convert_to_short_name(path, short_name);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }
    memcpy(entry.name, short_name, 11);

    /* Allocate cluster */
    uint32_t cluster;
    status = fat_driver_alloc_cluster();
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Update directory entry */
    entry.first_cluster_high = (uint16_t)(cluster >> 16);
    entry.first_cluster_low = (uint16_t)cluster;
    status = fat_driver_update_dir_entry(driver, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_delete_file(fat_driver_private_t *driver, const char *path)
{
    if (!driver || !path) {
        return FAT_ERROR_INVALID;
    }

    /* Find file entry */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(driver, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Free clusters */
    uint32_t cluster = (entry.first_cluster_high << 16) | entry.first_cluster_low;
    while (cluster < FAT_EOC(driver->config.fat_type)) {
        uint32_t next_cluster;
        status = fat_driver_get_next_cluster(driver, cluster, &next_cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }

        status = fat_driver_free_cluster(cluster);
        if (status != FAT_ERROR_SUCCESS) {
            return status;
        }

        cluster = next_cluster;
    }

    /* Mark directory entry as deleted */
    entry.name[0] = FAT_DIR_DELETED;
    status = fat_driver_update_dir_entry(driver, path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_rename_file(fat_driver_private_t *driver, const char *old_path, const char *new_path)
{
    if (!driver || !old_path || !new_path) {
        return FAT_ERROR_INVALID;
    }

    /* Find file entry */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(driver, old_path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Convert new name */
    char short_name[11];
    status = fat_driver_convert_to_short_name(new_path, short_name);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Update directory entry */
    memcpy(entry.name, short_name, 11);
    status = fat_driver_update_dir_entry(driver, old_path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

int32_t fat_driver_move_file(fat_driver_private_t *driver, const char *src_path, const char *dst_path)
{
    if (!driver || !src_path || !dst_path) {
        return FAT_ERROR_INVALID;
    }

    /* Find source file entry */
    fat_entry_t entry;
    int32_t status = fat_driver_find_dir_entry(driver, src_path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Convert destination name */
    char short_name[11];
    status = fat_driver_convert_to_short_name(dst_path, short_name);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    /* Update directory entry */
    memcpy(entry.name, short_name, 11);
    status = fat_driver_update_dir_entry(driver, src_path, &entry);
    if (status != FAT_ERROR_SUCCESS) {
        return status;
    }

    return FAT_ERROR_SUCCESS;
}

uint16_t fat_driver_get_time(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ((tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec >> 1));
}

uint16_t fat_driver_get_date(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return (((tm->tm_year - 80) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday);
}

int32_t fat_driver_convert_to_short_name(const char *long_name, char *short_name)
{
    if (!long_name || !short_name) {
        return FAT_ERROR_INVALID;
    }

    /* Get base name */
    const char *base = strrchr(long_name, '/');
    if (base) {
        base++;
    } else {
        base = long_name;
    }

    /* Get extension */
    const char *ext = strrchr(base, '.');
    if (ext) {
        ext++;
    }

    /* Copy base name */
    memset(short_name, ' ', 11);
    uint32_t i;
    for (i = 0; i < 8 && base[i] && base[i] != '.'; i++) {
        short_name[i] = toupper(base[i]);
    }

    /* Copy extension */
    if (ext) {
        for (i = 0; i < 3 && ext[i]; i++) {
            short_name[8 + i] = toupper(ext[i]);
        }
    }

    return FAT_ERROR_SUCCESS;
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 