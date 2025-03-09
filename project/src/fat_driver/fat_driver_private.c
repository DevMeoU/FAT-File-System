/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm private của module FAT Driver.
 *   Các hàm này chỉ được sử dụng trong module.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#define _GNU_SOURCE  /* For strdup */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "../common/common_types.h"
#include "fat_driver_types.h"
#include "fat_driver_private.h"
#include "fat_driver.h"
#include "../hal/hal.h"
#include "../hal/hal_storage.h"

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
static int32_t read_sector(uint32_t sector, uint8_t *buffer);
static int32_t write_sector(uint32_t sector, const uint8_t *buffer);
static uint8_t fat_calculate_short_name_checksum(const char *short_name);
int32_t calculate_layout(const fat_boot_sector_t *boot_sector);
uint32_t get_fat_size(const fat_boot_sector_t *boot_sector);
int32_t fat_find_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_create_file(const char *path, fat_dir_entry_t *entry);
int32_t fat_write_dir_entry(const fat_dir_entry_t *entry);
uint32_t fat_alloc_cluster(void);
int32_t fat_deinit(void);
int32_t fat_free_cluster(uint32_t cluster);

/*********************************************************************
 * Private Variables
 *********************************************************************/
static hal_device_info_t device_info;

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t read_sector(uint32_t sector, uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID;
    }

    /* Kiểm tra cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_ctx.cache[i].valid && fat_ctx.cache[i].sector == sector) {
            memcpy(buffer, fat_ctx.cache[i].data, FAT_SECTOR_SIZE);
            return STATUS_SUCCESS;
        }
    }

    /* Đọc từ HAL */
    int32_t status = hal_read_sector(sector, buffer);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Cập nhật cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (!fat_ctx.cache[i].valid) {
            fat_ctx.cache[i].valid = true;
            fat_ctx.cache[i].sector = sector;
            fat_ctx.cache[i].dirty = false;
            memcpy(fat_ctx.cache[i].data, buffer, FAT_SECTOR_SIZE);
            break;
        }
    }

    return STATUS_SUCCESS;
}

static int32_t write_sector(uint32_t sector, const uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID;
    }

    /* Kiểm tra cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_ctx.cache[i].valid && fat_ctx.cache[i].sector == sector) {
            memcpy(fat_ctx.cache[i].data, buffer, FAT_SECTOR_SIZE);
            fat_ctx.cache[i].dirty = true;
            return STATUS_SUCCESS;
        }
    }

    /* Ghi vào HAL */
    return hal_write_sector(sector, buffer);
}

int32_t fat_init_storage(void)
{
    int32_t ret;

    /* Get device info */
    ret = hal_get_device_info(&device_info);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    /* Validate sector size */
    if (STORAGE_MAX_SECTOR_SIZE != FAT_SECTOR_SIZE) {
        return STATUS_INVALID;
    }

    return STATUS_SUCCESS;
}

int32_t fat_read_boot_sector(void)
{
    uint8_t buffer[FAT_SECTOR_SIZE];
    int32_t ret;

    /* Read boot sector */
    ret = read_sector(0, buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    /* Copy to boot sector structure */
    memcpy(&boot_sector, buffer, sizeof(fat_boot_sector_t));

    return STATUS_SUCCESS;
}

int32_t fat_validate_boot_sector(void)
{
    /* Check signature */
    if (boot_sector.signature[0] != 0x55 ||
        boot_sector.signature[1] != 0xAA) {
        return STATUS_INVALID;
    }

    /* Check sector size */
    if (boot_sector.bytes_per_sector != FAT_SECTOR_SIZE) {
        return STATUS_INVALID;
    }

    /* Check sectors per cluster */
    if (boot_sector.sectors_per_cluster == 0 ||
        boot_sector.sectors_per_cluster > 128) {
        return STATUS_INVALID;
    }

    /* Check reserved sectors */
    if (boot_sector.reserved_sectors == 0) {
        return STATUS_INVALID;
    }

    /* Check number of FATs */
    if (boot_sector.num_fats == 0 ||
        boot_sector.num_fats > 2) {
        return STATUS_INVALID;
    }

    /* Check root directory entries */
    if (boot_sector.root_entries == 0) {
        return STATUS_INVALID;
    }

    /* Check total sectors */
    if (boot_sector.total_sectors_16 == 0 &&
        boot_sector.total_sectors_32 == 0) {
        return STATUS_INVALID;
    }

    return STATUS_SUCCESS;
}

int32_t fat_init_fat_tables(void)
{
    uint8_t buffer[FAT_SECTOR_SIZE];
    int32_t ret;

    /* Read first FAT */
    ret = read_sector(boot_sector.reserved_sectors, buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    /* Check FAT type */
    uint16_t first_cluster = *(uint16_t *)&buffer[0x1FE];
    if (first_cluster == 0xFFF) {
        fat_type = FAT_TYPE_12;
    } else if (first_cluster == 0xFFFF) {
        fat_type = FAT_TYPE_16;
    } else {
        /* For FAT32, we need to read 4 bytes */
        uint32_t fat32_cluster = *(uint32_t *)&buffer[0x1FC];
        if (fat32_cluster == 0x0FFFFFFF) {
            fat_type = FAT_TYPE_32;
        } else {
            return STATUS_INVALID;
        }
    }

    return STATUS_SUCCESS;
}

static uint8_t fat_calculate_short_name_checksum(const char *short_name)
{
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    }
    return sum;
}

int32_t calculate_layout(const fat_boot_sector_t *boot_sector)
{
    /* Calculate sectors per cluster */
    fat_ctx.config.sectors_per_cluster = boot_sector->sectors_per_cluster;

    /* Calculate reserved sectors */
    fat_ctx.config.reserved_sectors = boot_sector->reserved_sectors;

    /* Calculate root directory sectors */
    fat_ctx.config.root_dir_sectors = ((boot_sector->root_entries * 32) + 
                                      (FAT_SECTOR_SIZE - 1)) / FAT_SECTOR_SIZE;

    /* Calculate first data sector */
    fat_ctx.config.first_data_sector = fat_ctx.config.reserved_sectors +
                                      (boot_sector->num_fats * get_fat_size(boot_sector)) +
                                      fat_ctx.config.root_dir_sectors;

    /* Calculate total clusters */
    uint32_t total_sectors = boot_sector->total_sectors_16;
    if (total_sectors == 0) {
        total_sectors = boot_sector->total_sectors_32;
    }
    fat_ctx.config.total_clusters = (total_sectors - fat_ctx.config.first_data_sector) /
                                   fat_ctx.config.sectors_per_cluster;

    /* Set FAT start and size */
    fat_ctx.fat_start = boot_sector->reserved_sectors;
    fat_ctx.fat_size = get_fat_size(boot_sector);

    /* Set root cluster */
    fat_ctx.root_cluster = boot_sector->root_cluster;

    return STATUS_SUCCESS;
}

uint32_t get_fat_size(const fat_boot_sector_t *boot_sector)
{
    uint32_t total_sectors = boot_sector->total_sectors_16;
    if (total_sectors == 0) {
        total_sectors = boot_sector->total_sectors_32;
    }

    uint32_t fat_size = (total_sectors * boot_sector->sectors_per_cluster * 2) / FAT_SECTOR_SIZE;
    if (fat_size % 2 != 0) {
        fat_size++;
    }

    return fat_size;
}

int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
{
    return read_sector(sector, buffer);
}

int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
{
    return write_sector(sector, buffer);
}

int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster)
{
    uint8_t buffer[FAT_SECTOR_SIZE];
    int32_t ret;

    /* Calculate FAT offset */
    uint32_t fat_offset;
    if (fat_type == FAT_TYPE_12) {
        fat_offset = cluster + (cluster / 2) * 3;
    } else if (fat_type == FAT_TYPE_16) {
        fat_offset = cluster * 2;
    } else {
        fat_offset = cluster * 4;
    }

    /* Calculate FAT sector */
    uint32_t fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
    uint32_t fat_sector_offset = fat_offset % FAT_SECTOR_SIZE;

    /* Read FAT sector */
    ret = read_sector(fat_sector, buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    /* Get next cluster */
    if (fat_type == FAT_TYPE_12) {
        if (cluster % 2 == 0) {
            *next_cluster = (buffer[fat_sector_offset + 1] << 4) |
                           (buffer[fat_sector_offset] & 0x0F);
        } else {
            *next_cluster = (buffer[fat_sector_offset + 2] << 8) |
                           buffer[fat_sector_offset + 1];
        }
    } else if (fat_type == FAT_TYPE_16) {
        *next_cluster = (buffer[fat_sector_offset + 1] << 8) |
                       buffer[fat_sector_offset];
    } else {
        *next_cluster = (buffer[fat_sector_offset + 3] << 24) |
                       (buffer[fat_sector_offset + 2] << 16) |
                       (buffer[fat_sector_offset + 1] << 8) |
                       buffer[fat_sector_offset];
    }

    return STATUS_SUCCESS;
}

int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster)
{
    uint8_t buffer[FAT_SECTOR_SIZE];
    int32_t ret;

    /* Calculate FAT offset */
    uint32_t fat_offset;
    if (fat_type == FAT_TYPE_12) {
        fat_offset = cluster + (cluster / 2) * 3;
    } else if (fat_type == FAT_TYPE_16) {
        fat_offset = cluster * 2;
    } else {
        fat_offset = cluster * 4;
    }

    /* Calculate FAT sector */
    uint32_t fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
    uint32_t fat_sector_offset = fat_offset % FAT_SECTOR_SIZE;

    /* Read FAT sector */
    ret = read_sector(fat_sector, buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    /* Update next cluster */
    if (fat_type == FAT_TYPE_12) {
        if (cluster % 2 == 0) {
            buffer[fat_sector_offset] = (buffer[fat_sector_offset] & 0xF0) |
                                      (next_cluster & 0x0F);
            buffer[fat_sector_offset + 1] = (next_cluster >> 4) & 0xFF;
        } else {
            buffer[fat_sector_offset + 1] = next_cluster & 0xFF;
            buffer[fat_sector_offset + 2] = (next_cluster >> 8) & 0x0F;
        }
    } else if (fat_type == FAT_TYPE_16) {
        buffer[fat_sector_offset] = next_cluster & 0xFF;
        buffer[fat_sector_offset + 1] = (next_cluster >> 8) & 0xFF;
    } else {
        buffer[fat_sector_offset] = next_cluster & 0xFF;
        buffer[fat_sector_offset + 1] = (next_cluster >> 8) & 0xFF;
        buffer[fat_sector_offset + 2] = (next_cluster >> 16) & 0xFF;
        buffer[fat_sector_offset + 3] = (next_cluster >> 24) & 0xFF;
    }

    /* Write FAT sector */
    return write_sector(fat_sector, buffer);
}

int32_t fat_find_free_cluster(uint32_t *cluster)
{
    int32_t ret;

    /* Start from cluster 2 */
    for (uint32_t i = 2; i < fat_ctx.config.total_clusters; i++) {
        uint32_t next_cluster;
        ret = fat_read_fat_entry(i, &next_cluster);
        if (ret != STATUS_SUCCESS) {
            return ret;
        }

        if (next_cluster == 0) {
            *cluster = i;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_NO_SPACE;
}

int32_t fat_convert_to_short_name(const char *name, char *short_name)
{
    char *ext = strrchr(name, '.');
    char *base = strdup(name);
    if (ext) {
        base[ext - name] = '\0';
    }

    /* Convert base name */
    int i;
    for (i = 0; i < 8 && base[i] != '\0'; i++) {
        short_name[i] = toupper(base[i]);
    }
    for (; i < 8; i++) {
        short_name[i] = ' ';
    }

    /* Convert extension */
    if (ext) {
        for (i = 0; i < 3 && ext[i + 1] != '\0'; i++) {
            short_name[8 + i] = toupper(ext[i + 1]);
        }
        for (; i < 3; i++) {
            short_name[8 + i] = ' ';
        }
    } else {
        for (i = 0; i < 3; i++) {
            short_name[8 + i] = ' ';
        }
    }

    free(base);
    return STATUS_SUCCESS;
}

void fat_get_name(const fat_dir_entry_t *entry, char *name)
{
    /* Copy base name */
    int i;
    for (i = 0; i < 8 && entry->name[i] != ' '; i++) {
        name[i] = entry->name[i];
    }

    /* Add extension if present */
    if (entry->name[8] != ' ') {
        name[i++] = '.';
        for (int j = 0; j < 3 && entry->name[8 + j] != ' '; j++) {
            name[i++] = entry->name[8 + j];
        }
    }

    name[i] = '\0';
}

int32_t fat_find_file(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return STATUS_INVALID;
    }

    /* TODO: Implement file search */
    return STATUS_SUCCESS;
}

int32_t fat_create_file(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return STATUS_INVALID;
    }

    /* TODO: Implement file creation */
    return STATUS_SUCCESS;
}

int32_t fat_write_dir_entry(const fat_dir_entry_t *entry)
{
    if (!entry) {
        return STATUS_INVALID;
    }

    /* TODO: Implement directory entry writing */
    return STATUS_SUCCESS;
}

uint32_t fat_alloc_cluster(void)
{
    /* TODO: Implement cluster allocation */
    return 0;
}

int32_t fat_deinit(void)
{
    /* TODO: Implement deinitialization */
    return STATUS_SUCCESS;
}

int32_t fat_free_cluster(uint32_t cluster)
{
    /* Mark cluster as free */
    return fat_write_fat_entry(cluster, 0);
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
