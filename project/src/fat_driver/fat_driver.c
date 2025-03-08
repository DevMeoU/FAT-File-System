/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module FAT Driver cung cấp các hàm để đọc/ghi dữ liệu trên hệ thống
 *   tập tin FAT (File Allocation Table). Hỗ trợ các phiên bản FAT12,
 *   FAT16 và FAT32.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common/common_type.h"
#include "fat_driver.h"
#include "fat_driver_private.h"
#include "../hal/hal.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
static fat_boot_sector_t boot_sector;
static uint32_t fat_type;
static uint32_t root_dir_sectors;
static uint32_t first_data_sector;
static uint32_t data_sectors;
static uint32_t total_clusters;
static fat_context_t fat_ctx;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
uint32_t get_first_sector(uint32_t cluster);
uint32_t get_next_cluster(uint32_t current_cluster);
int32_t read_sector(uint32_t sector, uint8_t *buffer);
int32_t write_sector(uint32_t sector, const uint8_t *buffer);
int32_t fat_read_sector(uint32_t sector, uint8_t *buffer);
int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer);
int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster);
int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster);

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t fat_init(const fat_config_t *config)
{
    if (!config) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    // Khởi tạo HAL
    hal_config_t hal_config = {
        .storage_type = HAL_STORAGE_IP,  // Sử dụng IP storage
        .buffer_size = FAT_BUFFER_SIZE,
        .timeout = HAL_DEFAULT_TIMEOUT,
        .interrupt_enable = false
    };
    
    int32_t ret = hal_init(&hal_config);
    if (ret != HAL_SUCCESS) {
        return FAT_ERROR;
    }

    // Khởi tạo context
    memset(&fat_ctx, 0, sizeof(fat_ctx));
    memcpy(&fat_ctx.config, config, sizeof(fat_config_t));

    // Đọc boot sector
    ret = fat_read_sector(0, (uint8_t *)&fat_ctx.boot);
    if (ret != FAT_SUCCESS) {
        return ret;
    }

    // Kiểm tra chữ ký boot sector
    if (fat_ctx.boot.signature != FAT_SIGNATURE_AA55) {
        return FAT_ERROR;
    }

    // Tính các thông số FAT
    fat_ctx.fat_start = fat_ctx.boot.reserved_sectors;
    fat_ctx.root_start = fat_ctx.fat_start + 
                        (fat_ctx.boot.fats * fat_ctx.boot.sectors_per_fat);
    fat_ctx.data_start = fat_ctx.root_start + 
                        ((fat_ctx.boot.root_entries * 32 + 
                          (FAT_SECTOR_SIZE - 1)) / FAT_SECTOR_SIZE);

    fat_ctx.mounted = true;
    return FAT_SUCCESS;
}

int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file)
{
    if (path == NULL || file == NULL) {
        return FAT_INVALID;
    }

    /* Initialize file handle */
    memset(file, 0, sizeof(fat_file_t));
    file->mode = mode;

    /* Parse path and find file */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status == FAT_NOT_FOUND) {
        if (mode & FAT_MODE_CREATE) {
            /* Create new file */
            status = fat_create_file(path, &entry);
            if (status != FAT_SUCCESS) {
                return status;
            }
        } else {
            return FAT_NOT_FOUND;
        }
    } else if (status != FAT_SUCCESS) {
        return status;
    }

    /* Check access mode */
    if ((entry.attributes & FAT_ATTR_READ_ONLY) && (mode & FAT_MODE_WRITE)) {
        return FAT_READ_ONLY;
    }

    /* Set file info */
    file->info.attributes = entry.attributes;
    file->info.size = entry.file_size;
    file->info.cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    file->info.date = entry.last_write_date;
    file->info.time = entry.last_write_time;
    strncpy((char *)file->info.name, (char *)entry.name, sizeof(file->info.name));

    /* Initialize position */
    file->position = 0;
    file->cluster = file->info.cluster;
    file->sector = get_first_sector(file->cluster);
    file->offset = 0;

    return FAT_SUCCESS;
}

int32_t fat_close(fat_file_t *file)
{
    if (file == NULL) {
        return FAT_INVALID;
    }

    /* Update file info if modified */
    if (file->modified) {
        fat_dir_entry_t entry;
        int32_t status = fat_find_file((char *)file->info.name, &entry);
        if (status == FAT_SUCCESS) {
            entry.file_size = file->info.size;
            entry.first_cluster_hi = (uint16_t)(file->info.cluster >> 16);
            entry.first_cluster_lo = (uint16_t)file->info.cluster;
            entry.last_write_date = file->info.date;
            entry.last_write_time = file->info.time;

            /* Write entry back to directory */
            status = fat_write_dir_entry(&entry);
            if (status != FAT_SUCCESS) {
                return status;
            }
        }
    }

    return FAT_SUCCESS;
}

int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (file == NULL || buffer == NULL || bytes_read == NULL) {
        return FAT_INVALID;
    }

    /* Check if end of file */
    if (file->position >= file->info.size) {
        *bytes_read = 0;
        return FAT_EOF;
    }

    /* Limit read size to remaining bytes */
    uint32_t remaining = file->info.size - file->position;
    if (size > remaining) {
        size = remaining;
    }

    /* Read data */
    uint32_t total_read = 0;
    uint8_t *buf = (uint8_t *)buffer;

    while (total_read < size) {
        /* Read current sector */
        uint8_t sector_buffer[FAT_SECTOR_SIZE];
        int32_t status = read_sector(file->sector, sector_buffer);
        if (status != FAT_SUCCESS) {
            *bytes_read = total_read;
            return status;
        }

        /* Copy data from sector */
        uint32_t sector_remaining = FAT_SECTOR_SIZE - file->offset;
        uint32_t bytes_to_read = size - total_read;
        if (bytes_to_read > sector_remaining) {
            bytes_to_read = sector_remaining;
        }

        memcpy(buf + total_read, sector_buffer + file->offset, bytes_to_read);
        total_read += bytes_to_read;
        file->position += bytes_to_read;
        file->offset += bytes_to_read;

        /* Move to next sector if needed */
        if (file->offset >= FAT_SECTOR_SIZE) {
            file->offset = 0;
            file->sector++;

            /* Move to next cluster if needed */
            if ((file->sector - get_first_sector(file->cluster)) >= boot_sector.sectors_per_cluster) {
                file->cluster = get_next_cluster(file->cluster);
                if (file->cluster == 0) {
                    break;  /* End of cluster chain */
                }
                file->sector = get_first_sector(file->cluster);
            }
        }
    }

    *bytes_read = total_read;
    return FAT_SUCCESS;
}

int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (file == NULL || buffer == NULL || bytes_written == NULL) {
        return FAT_INVALID;
    }

    /* Check write access */
    if (!(file->mode & FAT_MODE_WRITE)) {
        return FAT_READ_ONLY;
    }

    /* Write data */
    uint32_t total_written = 0;
    const uint8_t *buf = (const uint8_t *)buffer;

    while (total_written < size) {
        /* Write current sector */
        uint8_t sector_buffer[FAT_SECTOR_SIZE];
        int32_t status = read_sector(file->sector, sector_buffer);
        if (status != FAT_SUCCESS) {
            *bytes_written = total_written;
            return status;
        }

        /* Copy data to sector */
        uint32_t sector_remaining = FAT_SECTOR_SIZE - file->offset;
        uint32_t bytes_to_write = size - total_written;
        if (bytes_to_write > sector_remaining) {
            bytes_to_write = sector_remaining;
        }

        memcpy(sector_buffer + file->offset, buf + total_written, bytes_to_write);
        
        /* Write sector back */
        status = write_sector(file->sector, sector_buffer);
        if (status != FAT_SUCCESS) {
            *bytes_written = total_written;
            return status;
        }

        total_written += bytes_to_write;
        file->position += bytes_to_write;
        file->offset += bytes_to_write;

        /* Update file size if needed */
        if (file->position > file->info.size) {
            file->info.size = file->position;
            file->modified = true;
        }

        /* Move to next sector if needed */
        if (file->offset >= FAT_SECTOR_SIZE) {
            file->offset = 0;
            file->sector++;

            /* Move to next cluster if needed */
            if ((file->sector - get_first_sector(file->cluster)) >= boot_sector.sectors_per_cluster) {
                uint32_t next_cluster = get_next_cluster(file->cluster);
                if (next_cluster == 0) {
                    /* Allocate new cluster */
                    next_cluster = fat_alloc_cluster();
                    if (next_cluster == 0) {
                        *bytes_written = total_written;
                        return FAT_DISK_FULL;
                    }
                    fat_write_fat_entry(file->cluster, next_cluster);
                }
                file->cluster = next_cluster;
                file->sector = get_first_sector(file->cluster);
            }
        }
    }

    *bytes_written = total_written;
    return FAT_SUCCESS;
}

int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin)
{
    if (file == NULL) {
        return FAT_INVALID;
    }

    /* Calculate new position */
    int32_t new_pos;
    switch (origin) {
        case SEEK_SET:
            new_pos = offset;
            break;
        case SEEK_CUR:
            new_pos = file->position + offset;
            break;
        case SEEK_END:
            new_pos = file->info.size + offset;
            break;
        default:
            return FAT_INVALID;
    }

    /* Check bounds */
    if (new_pos < 0 || (uint32_t)new_pos > file->info.size) {
        return FAT_INVALID;
    }

    /* Seek to new position */
    if (new_pos == 0) {
        /* Seek to beginning */
        file->position = 0;
        file->cluster = file->info.cluster;
        file->sector = get_first_sector(file->cluster);
        file->offset = 0;
    } else {
        /* Seek to position */
        uint32_t cluster = file->info.cluster;
        uint32_t pos = 0;
        uint32_t bytes_per_cluster = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;

        while (pos + bytes_per_cluster <= (uint32_t)new_pos) {
            cluster = get_next_cluster(cluster);
            if (cluster == 0) {
                return FAT_INVALID;
            }
            pos += bytes_per_cluster;
        }

        file->position = new_pos;
        file->cluster = cluster;
        file->sector = get_first_sector(cluster) + ((new_pos - pos) / boot_sector.bytes_per_sector);
        file->offset = (new_pos - pos) % boot_sector.bytes_per_sector;
    }

    return FAT_SUCCESS;
}

int32_t fat_stat(const char *path, fat_file_info_t *info)
{
    if (path == NULL || info == NULL) {
        return FAT_INVALID;
    }

    /* Find file */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Fill info structure */
    info->attributes = entry.attributes;
    info->size = entry.file_size;
    info->cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    info->date = entry.last_write_date;
    info->time = entry.last_write_time;
    strncpy((char *)info->name, (char *)entry.name, sizeof(info->name));

    return FAT_SUCCESS;
}

int32_t fat_unlink(const char *path)
{
    if (path == NULL) {
        return FAT_INVALID;
    }

    /* Find file */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Check if directory */
    if (entry.attributes & FAT_ATTR_DIRECTORY) {
        return FAT_INVALID;
    }

    /* Mark entry as deleted */
    entry.name[0] = 0xE5;
    status = fat_write_dir_entry(&entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Free cluster chain */
    uint32_t cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    while (cluster != 0 && cluster != 0x0FFFFFF8) {
        uint32_t next = get_next_cluster(cluster);
        fat_free_cluster(cluster);
        cluster = next;
    }

    return FAT_SUCCESS;
}

int32_t fat_mkdir(const char *path)
{
    if (path == NULL) {
        return FAT_INVALID;
    }

    /* Check if already exists */
    fat_dir_entry_t entry;
    if (fat_find_file(path, &entry) == FAT_SUCCESS) {
        return FAT_EXISTS;
    }

    /* Create directory entry */
    memset(&entry, 0, sizeof(entry));
    memcpy(entry.name, path, sizeof(entry.name));
    entry.attributes = FAT_ATTR_DIRECTORY;
    
    /* Allocate first cluster */
    uint32_t cluster = fat_alloc_cluster();
    if (cluster == 0) {
        return FAT_DISK_FULL;
    }
    
    entry.first_cluster_hi = (uint16_t)(cluster >> 16);
    entry.first_cluster_lo = (uint16_t)cluster;

    /* Write directory entry */
    int32_t status = fat_write_dir_entry(&entry);
    if (status != FAT_SUCCESS) {
        fat_free_cluster(cluster);
        return status;
    }

    /* Initialize directory contents */
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    memset(sector_buffer, 0, sizeof(sector_buffer));
    
    /* Create . and .. entries */
    fat_dir_entry_t *dot = (fat_dir_entry_t *)sector_buffer;
    memset(dot, 0, sizeof(*dot));
    memcpy(dot->name, ".          ", 11);
    dot->attributes = FAT_ATTR_DIRECTORY;
    dot->first_cluster_hi = entry.first_cluster_hi;
    dot->first_cluster_lo = entry.first_cluster_lo;

    fat_dir_entry_t *dotdot = dot + 1;
    memset(dotdot, 0, sizeof(*dotdot));
    memcpy(dotdot->name, "..         ", 11);
    dotdot->attributes = FAT_ATTR_DIRECTORY;
    /* Parent cluster will be set by caller */

    /* Write first sector */
    status = write_sector(get_first_sector(cluster), sector_buffer);
    if (status != FAT_SUCCESS) {
        fat_free_cluster(cluster);
        return status;
    }

    return FAT_SUCCESS;
}

int32_t fat_rmdir(const char *path)
{
    if (path == NULL) {
        return FAT_INVALID;
    }

    /* Find directory */
    fat_dir_entry_t entry;
    int32_t status = fat_find_file(path, &entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Check if directory */
    if (!(entry.attributes & FAT_ATTR_DIRECTORY)) {
        return FAT_INVALID;
    }

    /* Check if empty */
    uint32_t cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    status = read_sector(get_first_sector(cluster), sector_buffer);
    if (status != FAT_SUCCESS) {
        return status;
    }

    fat_dir_entry_t *dir = (fat_dir_entry_t *)sector_buffer;
    for (uint32_t i = 2; i < FAT_SECTOR_SIZE/sizeof(fat_dir_entry_t); i++) {
        if (dir[i].name[0] != 0 && dir[i].name[0] != 0xE5) {
            return FAT_ERROR;  /* Directory not empty */
        }
    }

    /* Mark entry as deleted */
    entry.name[0] = 0xE5;
    status = fat_write_dir_entry(&entry);
    if (status != FAT_SUCCESS) {
        return status;
    }

    /* Free cluster chain */
    while (cluster != 0 && cluster != 0x0FFFFFF8) {
        uint32_t next = get_next_cluster(cluster);
        fat_free_cluster(cluster);
        cluster = next;
    }

    return FAT_SUCCESS;
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

uint32_t get_first_sector(uint32_t cluster)
{
    return ((cluster - 2) * boot_sector.sectors_per_cluster) + first_data_sector;
}

uint32_t get_next_cluster(uint32_t current_cluster)
{
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    uint32_t next_cluster;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    switch (fat_type) {
        case FAT_TYPE_12:
            fat_offset = current_cluster + (current_cluster / 2);
            fat_sector = boot_sector.reserved_sectors + (fat_offset / boot_sector.bytes_per_sector);
            ent_offset = fat_offset % boot_sector.bytes_per_sector;

            if (read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return 0;
            }

            if (ent_offset == (uint32_t)(boot_sector.bytes_per_sector - 1)) {
                /* Cluster entry spans two sectors */
                uint8_t next_sector_buffer[FAT_SECTOR_SIZE];
                if (read_sector(fat_sector + 1, next_sector_buffer) != FAT_SUCCESS) {
                    return 0;
                }

                if (current_cluster & 0x1) {
                    next_cluster = (sector_buffer[ent_offset] >> 4) | (next_sector_buffer[0] << 4);
                } else {
                    next_cluster = sector_buffer[ent_offset] | ((next_sector_buffer[0] & 0x0F) << 8);
                }
            } else {
                if (current_cluster & 0x1) {
                    next_cluster = (sector_buffer[ent_offset] >> 4) | (sector_buffer[ent_offset + 1] << 4);
                } else {
                    next_cluster = sector_buffer[ent_offset] | ((sector_buffer[ent_offset + 1] & 0x0F) << 8);
                }
            }
            break;

        case FAT_TYPE_16:
            fat_offset = current_cluster * 2;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / boot_sector.bytes_per_sector);
            ent_offset = fat_offset % boot_sector.bytes_per_sector;

            if (read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return 0;
            }

            next_cluster = *(uint16_t *)&sector_buffer[ent_offset];
            break;

        case FAT_TYPE_32:
            fat_offset = current_cluster * 4;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / boot_sector.bytes_per_sector);
            ent_offset = fat_offset % boot_sector.bytes_per_sector;

            if (read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return 0;
            }

            next_cluster = *(uint32_t *)&sector_buffer[ent_offset] & 0x0FFFFFFF;
            break;

        default:
            return 0;
    }

    return next_cluster;
}

int32_t read_sector(uint32_t sector, uint8_t *buffer)
{
    return (ip_read_sector(sector, buffer) == IP_SUCCESS) ? FAT_SUCCESS : FAT_ERROR;
}

int32_t write_sector(uint32_t sector, const uint8_t *buffer)
{
    return (ip_write_sector(sector, buffer) == IP_SUCCESS) ? FAT_SUCCESS : FAT_ERROR;
}

int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
{
    if (!buffer) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    #if FAT_ENABLE_CACHE
    // Kiểm tra cache
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_ctx.cache[i].valid && fat_ctx.cache[i].sector == sector) {
            memcpy(buffer, fat_ctx.cache[i].data, FAT_SECTOR_SIZE);
            return FAT_SUCCESS;
        }
    }
    #endif

    // Đọc từ thiết bị thông qua HAL
    int32_t ret = hal_read_sector(sector, buffer);
    if (ret != HAL_SUCCESS) {
        return FAT_ERROR_READ_FAILED;
    }

    #if FAT_ENABLE_CACHE
    // Cập nhật cache
    uint32_t cache_index = sector % FAT_CACHE_SIZE;
    fat_ctx.cache[cache_index].sector = sector;
    memcpy(fat_ctx.cache[cache_index].data, buffer, FAT_SECTOR_SIZE);
    fat_ctx.cache[cache_index].valid = true;
    fat_ctx.cache[cache_index].dirty = false;
    #endif

    return FAT_SUCCESS;
}

int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
{
    if (!buffer) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    // Ghi xuống thiết bị thông qua HAL
    int32_t ret = hal_write_sector(sector, buffer);
    if (ret != HAL_SUCCESS) {
        return FAT_ERROR_WRITE_FAILED;
    }

    #if FAT_ENABLE_CACHE
    // Cập nhật cache
    uint32_t cache_index = sector % FAT_CACHE_SIZE;
    fat_ctx.cache[cache_index].sector = sector;
    memcpy(fat_ctx.cache[cache_index].data, buffer, FAT_SECTOR_SIZE);
    fat_ctx.cache[cache_index].valid = true;
    fat_ctx.cache[cache_index].dirty = false;
    #endif

    return FAT_SUCCESS;
}

int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster)
{
    if (!next_cluster) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    // Tính vị trí entry trong FAT
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = fat_ctx.fat_start + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = fat_ctx.fat_start + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = fat_ctx.fat_start + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        default:
            return FAT_ERROR_INVALID_PARAMETER;
    }

    // Đọc sector chứa entry
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    int32_t ret = fat_read_sector(fat_sector, sector_buffer);
    if (ret != FAT_SUCCESS) {
        return ret;
    }

    // Đọc giá trị entry
    uint32_t entry_value;
    
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            if (cluster & 0x1) {
                entry_value = (sector_buffer[ent_offset] >> 4) |
                            (sector_buffer[ent_offset + 1] << 4);
            } else {
                entry_value = sector_buffer[ent_offset] |
                            ((sector_buffer[ent_offset + 1] & 0x0F) << 8);
            }
            entry_value &= FAT12_MASK;
            break;
            
        case FAT_TYPE_16:
            entry_value = sector_buffer[ent_offset] |
                         (sector_buffer[ent_offset + 1] << 8);
            entry_value &= FAT16_MASK;
            break;
            
        case FAT_TYPE_32:
            entry_value = sector_buffer[ent_offset] |
                         (sector_buffer[ent_offset + 1] << 8) |
                         (sector_buffer[ent_offset + 2] << 16) |
                         (sector_buffer[ent_offset + 3] << 24);
            entry_value &= FAT32_MASK;
            break;
            
        default:
            return FAT_ERROR_INVALID_PARAMETER;
    }

    *next_cluster = entry_value;
    return FAT_SUCCESS;
}

int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster)
{
    // Tính vị trí entry trong FAT
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = fat_ctx.fat_start + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = fat_ctx.fat_start + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = fat_ctx.fat_start + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        default:
            return FAT_ERROR_INVALID_PARAMETER;
    }

    // Đọc sector chứa entry
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    int32_t ret = fat_read_sector(fat_sector, sector_buffer);
    if (ret != FAT_SUCCESS) {
        return ret;
    }

    // Ghi giá trị entry
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            if (cluster & 0x1) {
                sector_buffer[ent_offset] = (sector_buffer[ent_offset] & 0x0F) |
                                          ((next_cluster & 0x0F) << 4);
                sector_buffer[ent_offset + 1] = (next_cluster >> 4) & 0xFF;
            } else {
                sector_buffer[ent_offset] = next_cluster & 0xFF;
                sector_buffer[ent_offset + 1] = (sector_buffer[ent_offset + 1] & 0xF0) |
                                              ((next_cluster >> 8) & 0x0F);
            }
            break;
            
        case FAT_TYPE_16:
            sector_buffer[ent_offset] = next_cluster & 0xFF;
            sector_buffer[ent_offset + 1] = (next_cluster >> 8) & 0xFF;
            break;
            
        case FAT_TYPE_32:
            sector_buffer[ent_offset] = next_cluster & 0xFF;
            sector_buffer[ent_offset + 1] = (next_cluster >> 8) & 0xFF;
            sector_buffer[ent_offset + 2] = (next_cluster >> 16) & 0xFF;
            sector_buffer[ent_offset + 3] = (next_cluster >> 24) & 0xFF;
            break;
            
        default:
            return FAT_ERROR_INVALID_PARAMETER;
    }

    // Ghi sector đã cập nhật
    ret = fat_write_sector(fat_sector, sector_buffer);
    if (ret != FAT_SUCCESS) {
        return ret;
    }

    return FAT_SUCCESS;
}

/*********************************************************************
 * UUID: 4b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
