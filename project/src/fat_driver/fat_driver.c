#define _GNU_SOURCE  /* For strdup */
/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Triển khai FAT File System module, hỗ trợ FAT12/16/32.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "../common/common_types.h"
#include "fat_driver_types.h"
#include "fat_driver_private.h"
#include "../hal/hal.h"
#include "fat_driver.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
fat_context_t fat_ctx;
fat_boot_sector_t boot_sector;
uint32_t fat_type;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
static int32_t read_sector(uint32_t sector, uint8_t *buffer);
static int32_t write_sector(uint32_t sector, const uint8_t *buffer);
static uint32_t get_fat_size(const fat_boot_sector_t *boot_sector);
static void calculate_layout(const fat_boot_sector_t *boot_sector);
static uint32_t fat_cluster_to_sector(uint32_t cluster);
static uint16_t fat_get_time(void);
static uint16_t fat_get_date(void);
static void fat_get_name(const fat_dir_entry_t *entry, char *name);
static uint32_t get_next_cluster(uint32_t current_cluster);
static uint8_t fat_calculate_short_name_checksum(const char *short_name);
int32_t fat_read_sector(uint32_t sector, uint8_t *buffer);
int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer);
int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster);
int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster);

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t fat_init(const fat_config_t *config) {
    if (!config) {
        return STATUS_INVALID;
    }
    
    /* Kiểm tra xem đã mount chưa */
    if (fat_ctx.mounted) {
        return STATUS_SUCCESS;
    }
    
    /* Khởi tạo HAL */
    hal_config_t hal_config = {
        .file_path = config->file_path,
        .base_addr = config->base_addr,
        .irq_num = config->irq_num,
        .use_dma = config->use_dma
    };
    
    int32_t status = hal_init(&hal_config);
    if (status != STATUS_SUCCESS) {
        return status;
    }
    
    /* Đọc boot sector */
    if (read_sector(0, (uint8_t *)&boot_sector) != STATUS_SUCCESS) {
        hal_deinit();
        return STATUS_READ_FAILED;
    }
    
    /* Kiểm tra signature và các thông số cơ bản */
    if (boot_sector.signature != FAT_SIGNATURE_AA55 ||
        boot_sector.bytes_per_sector != FAT_SECTOR_SIZE ||
        boot_sector.sectors_per_cluster == 0 ||
        boot_sector.reserved_sectors == 0 ||
        boot_sector.num_fats == 0 ||
        boot_sector.root_entries == 0) {
        hal_deinit();
        return STATUS_INVALID;
    }

    /* Kiểm tra total sectors */
    uint32_t total_sectors;
    if (boot_sector.total_sectors_16 != 0) {
        total_sectors = boot_sector.total_sectors_16;
    } else {
        total_sectors = boot_sector.total_sectors_32;
    }
    
    if (total_sectors == 0) {
        hal_deinit();
        return STATUS_INVALID;
    }

    /* Kiểm tra boot signature */
    if (boot_sector.boot_signature != 0x28 && boot_sector.boot_signature != 0x29) {
        hal_deinit();
        return STATUS_INVALID;
    }

    /* Kiểm tra volume label */
    if (boot_sector.volume_label[0] == 0x00) {
        /* Nếu không có volume label, set mặc định là "NO NAME    " */
        memcpy(boot_sector.volume_label, "NO NAME    ", 11);
    }
    
    /* Tính toán layout */
    calculate_layout(&boot_sector);
    
    /* Khởi tạo cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        fat_ctx.cache[i].valid = false;
        fat_ctx.cache[i].dirty = false;
    }
    
    /* Đánh dấu đã mount */
    fat_ctx.mounted = true;
    
    return STATUS_SUCCESS;
}

int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file)
{
    if (path == NULL || file == NULL) {
        return STATUS_INVALID;
    }

    /* Initialize file handle */
    memset(file, 0, sizeof(fat_file_t));
    file->mode = mode;

    /* Tìm và tính entry cho file */
    fat_dir_entry_t entry;
    memset(&entry, 0, sizeof(fat_dir_entry_t));
    
    /* Tìm file trong thư mục */
    int32_t status = fat_find_file(path, &entry);
    
    /* Xử lý trường hợp file không tồn tại */
    if (status == STATUS_NOT_FOUND) {
        if (mode & FAT_MODE_CREATE) {
            /* Tạo file mới nếu có quyền tạo */
            status = fat_create_file(path, &entry);
            if (status != STATUS_SUCCESS) {
                return status; /* Lỗi khi tạo file */
            }
        } else {
            return STATUS_NOT_FOUND; /* Không tìm thấy file */
        }
    } else if (status != STATUS_SUCCESS) {
        return status; /* Lỗi khác */
    }

    /* Check access mode */
    if ((entry.attributes & FAT_ATTR_READ_ONLY) && (mode & FAT_MODE_WRITE)) {
        return STATUS_READ_ONLY;
    }

    /* Set file info */
    file->info.attributes = entry.attributes;
    file->info.size = entry.file_size;
    file->info.cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    file->info.date = fat_get_date();
    file->info.time = fat_get_time();
    fat_get_name(&entry, (char *)file->info.name);

    /* Initialize position */
    file->position = 0;
    file->cluster = file->info.cluster;
    file->sector = fat_cluster_to_sector(file->cluster);
    file->offset = 0;

    return STATUS_SUCCESS;
}

int32_t fat_close(fat_file_t *file)
{
    if (file == NULL) {
        return STATUS_INVALID;
    }

    /* Update file info if modified */
    if (file->modified) {
        fat_dir_entry_t entry;
        int32_t status = fat_find_file((char *)file->info.name, &entry);
        if (status == STATUS_SUCCESS) {
            entry.file_size = file->info.size;
            entry.first_cluster_hi = (uint16_t)(file->info.cluster >> 16);
            entry.first_cluster_lo = (uint16_t)file->info.cluster;
            entry.last_write_date = file->info.date;
            entry.last_write_time = file->info.time;

            /* Write entry back to directory */
            status = fat_write_dir_entry(&entry);
            if (status != STATUS_SUCCESS) {
                return status;
            }
        }
    }

    return STATUS_SUCCESS;
}

int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read)
{
    if (file == NULL || buffer == NULL || bytes_read == NULL) {
        return STATUS_INVALID;
    }

    /* Check if end of file */
    if (file->position >= file->info.size) {
        *bytes_read = 0;
        return STATUS_EOF;
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
        if (status != STATUS_SUCCESS) {
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
            if ((file->sector - fat_cluster_to_sector(file->cluster)) >= boot_sector.sectors_per_cluster) {
                file->cluster = get_next_cluster(file->cluster);
                if (file->cluster == 0) {
                    break;  /* End of cluster chain */
                }
                file->sector = fat_cluster_to_sector(file->cluster);
            }
        }
    }

    *bytes_read = total_read;
    return STATUS_SUCCESS;
}

int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written)
{
    if (file == NULL || buffer == NULL || bytes_written == NULL) {
        return STATUS_INVALID;
    }

    /* Check write access */
    if (!(file->mode & FAT_MODE_WRITE)) {
        return STATUS_READ_ONLY;
    }

    /* Write data */
    uint32_t total_written = 0;
    const uint8_t *buf = (const uint8_t *)buffer;

    while (total_written < size) {
        /* Write current sector */
        uint8_t sector_buffer[FAT_SECTOR_SIZE];
        int32_t status = read_sector(file->sector, sector_buffer);
        if (status != STATUS_SUCCESS) {
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
        if (status != STATUS_SUCCESS) {
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
            if ((file->sector - fat_cluster_to_sector(file->cluster)) >= boot_sector.sectors_per_cluster) {
                uint32_t next_cluster = get_next_cluster(file->cluster);
                if (next_cluster == 0) {
                    /* Allocate new cluster */
                    next_cluster = fat_alloc_cluster();
                    if (next_cluster == 0) {
                        *bytes_written = total_written;
                        return STATUS_DISK_FULL;
                    }
                    fat_write_fat_entry(file->cluster, next_cluster);
                }
                file->cluster = next_cluster;
                file->sector = fat_cluster_to_sector(file->cluster);
            }
        }
    }

    *bytes_written = total_written;
    return STATUS_SUCCESS;
}

int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin)
{
    if (file == NULL) {
        return STATUS_INVALID;
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
            return STATUS_INVALID;
    }

    /* Check bounds */
    if (new_pos < 0 || (uint32_t)new_pos > file->info.size) {
        return STATUS_INVALID;
    }

    /* Seek to new position */
    if (new_pos == 0) {
        /* Seek to beginning */
        file->position = 0;
        file->cluster = file->info.cluster;
        file->sector = fat_cluster_to_sector(file->cluster);
        file->offset = 0;
    } else {
        /* Seek to position */
        uint32_t cluster = file->info.cluster;
        uint32_t pos = 0;
        uint32_t bytes_per_cluster = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;

        while (pos + bytes_per_cluster <= (uint32_t)new_pos) {
            cluster = get_next_cluster(cluster);
            if (cluster == 0) {
                return STATUS_INVALID;
            }
            pos += bytes_per_cluster;
        }

        file->position = new_pos;
        file->cluster = cluster;
        file->sector = fat_cluster_to_sector(cluster) + ((new_pos - pos) / boot_sector.bytes_per_sector);
        file->offset = (new_pos - pos) % boot_sector.bytes_per_sector;
    }

    return STATUS_SUCCESS;
}

int32_t fat_stat(const char *path, fat_dir_entry_t *info)
{
    if (path == NULL || info == NULL) {
        return STATUS_INVALID;
    }

    /* Find file and fill info structure directly */
    return fat_find_file(path, info);
}

int32_t fat_unlink(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Find file */
    fat_dir_entry_t entry;
    memset(&entry, 0, sizeof(fat_dir_entry_t));
    int32_t status = fat_find_file(path, &entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Check if directory */
    if (entry.attributes & FAT_ATTR_DIRECTORY) {
        return STATUS_INVALID;
    }

    /* Mark entry as deleted */
    entry.name[0] = 0xE5;
    status = fat_write_dir_entry(&entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Free cluster chain */
    uint32_t cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    while (cluster != 0 && cluster != 0x0FFFFFF8) {
        uint32_t next = get_next_cluster(cluster);
        fat_free_cluster(cluster);
        cluster = next;
    }

    return STATUS_SUCCESS;
}

int32_t fat_mkdir(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Check if already exists */
    fat_dir_entry_t entry;
    if (fat_find_file(path, &entry) == STATUS_SUCCESS) {
        return STATUS_EXISTS;
    }

    /* Create directory entry */
    memset(&entry, 0, sizeof(entry));
    memcpy(entry.name, path, sizeof(entry.name));
    entry.attributes = FAT_ATTR_DIRECTORY;
    entry.last_write_date = fat_get_date();
    entry.last_write_time = fat_get_time();
    
    /* Allocate first cluster */
    uint32_t cluster = fat_alloc_cluster();
    if (cluster == 0) {
        return STATUS_DISK_FULL;
    }
    
    entry.first_cluster_hi = (uint16_t)(cluster >> 16);
    entry.first_cluster_lo = (uint16_t)cluster;

    /* Write directory entry */
    int32_t status = fat_write_dir_entry(&entry);
    if (status != STATUS_SUCCESS) {
        fat_free_cluster(cluster);
        return status;
    }

    return STATUS_SUCCESS;
}

int32_t fat_rmdir(const char *path)
{
    if (path == NULL) {
        return STATUS_INVALID;
    }

    /* Find directory */
    fat_dir_entry_t entry;
    memset(&entry, 0, sizeof(fat_dir_entry_t));
    int32_t status = fat_find_file(path, &entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Check if directory */
    if (!(entry.attributes & FAT_ATTR_DIRECTORY)) {
        return STATUS_INVALID;
    }

    /* Check if empty */
    uint32_t cluster = (entry.first_cluster_hi << 16) | entry.first_cluster_lo;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    status = read_sector(fat_cluster_to_sector(cluster), sector_buffer);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    fat_dir_entry_t *dir = (fat_dir_entry_t *)sector_buffer;
    for (uint32_t i = 2; i < FAT_SECTOR_SIZE/sizeof(fat_dir_entry_t); i++) {
        if (dir[i].name[0] != 0 && dir[i].name[0] != 0xE5) {
            return STATUS_ERROR;  /* Directory not empty */
        }
    }

    /* Mark entry as deleted */
    entry.name[0] = 0xE5;
    status = fat_write_dir_entry(&entry);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Free cluster chain */
    while (cluster != 0 && cluster != 0x0FFFFFF8) {
        uint32_t next = get_next_cluster(cluster);
        fat_free_cluster(cluster);
        cluster = next;
    }

    return STATUS_SUCCESS;
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static uint32_t get_next_cluster(uint32_t current_cluster)
{
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    uint32_t next_cluster;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    switch (fat_type) {
        case FAT_TYPE_12:
            /* Công thức cho FAT12: offset = cluster * 1.5 */
            fat_offset = current_cluster + (current_cluster / 2);
            fat_sector = boot_sector.reserved_sectors + (fat_offset / boot_sector.bytes_per_sector);
            ent_offset = fat_offset % boot_sector.bytes_per_sector;

            if (read_sector(fat_sector, sector_buffer) != STATUS_SUCCESS) {
                return 0;
            }

            /* Đọc 12-bit entry */
            if (current_cluster & 0x1) {
                /* Entry lẻ: lấy 4 bit cao của byte hiện tại và 8 bit thấp của byte tiếp theo */
                next_cluster = (sector_buffer[ent_offset] >> 4) | (sector_buffer[ent_offset + 1] << 4);
            } else {
                /* Entry chẵn: lấy 8 bit thấp của byte hiện tại và 4 bit thấp của byte tiếp theo */
                next_cluster = sector_buffer[ent_offset] | ((sector_buffer[ent_offset + 1] & 0x0F) << 8);
            }
            next_cluster &= FAT12_MASK;
            break;

        case FAT_TYPE_16:
            /* Công thức cho FAT16: offset = cluster * 2 */
            fat_offset = current_cluster * 2;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / boot_sector.bytes_per_sector);
            ent_offset = fat_offset % boot_sector.bytes_per_sector;

            if (read_sector(fat_sector, sector_buffer) != STATUS_SUCCESS) {
                return 0;
            }

            next_cluster = *(uint16_t *)&sector_buffer[ent_offset];
            next_cluster &= FAT16_MASK;
            break;

        case FAT_TYPE_32:
            /* Công thức cho FAT32: offset = cluster * 4 */
            fat_offset = current_cluster * 4;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / boot_sector.bytes_per_sector);
            ent_offset = fat_offset % boot_sector.bytes_per_sector;

            if (read_sector(fat_sector, sector_buffer) != STATUS_SUCCESS) {
                return 0;
            }

            next_cluster = *(uint32_t *)&sector_buffer[ent_offset] & FAT32_MASK;
            break;

        default:
            return 0;
    }

    return next_cluster;
}

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

    /* Ghi vào HAL */
    int32_t status = hal_write_sector(sector, buffer);
    if (status != STATUS_SUCCESS) {
        return status;
    }

    /* Cập nhật cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_ctx.cache[i].valid && fat_ctx.cache[i].sector == sector) {
            memcpy(fat_ctx.cache[i].data, buffer, FAT_SECTOR_SIZE);
            fat_ctx.cache[i].dirty = false;
            break;
        }
    }

    return STATUS_SUCCESS;
}

int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID;
    }

    #if FAT_ENABLE_CACHE
    // Kiểm tra cache
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_ctx.cache[i].valid && fat_ctx.cache[i].sector == sector) {
            memcpy(buffer, fat_ctx.cache[i].data, FAT_SECTOR_SIZE);
            return STATUS_SUCCESS;
        }
    }
    #endif

    // Đọc từ thiết bị thông qua HAL
    int32_t ret = hal_read_sector(sector, buffer);
    if (ret != STATUS_SUCCESS) {
        return STATUS_READ_FAILED;
    }

    #if FAT_ENABLE_CACHE
    // Cập nhật cache
    uint32_t cache_index = sector % FAT_CACHE_SIZE;
    fat_ctx.cache[cache_index].sector = sector;
    memcpy(fat_ctx.cache[cache_index].data, buffer, FAT_SECTOR_SIZE);
    fat_ctx.cache[cache_index].valid = true;
    fat_ctx.cache[cache_index].dirty = false;
    #endif

    return STATUS_SUCCESS;
}

int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID;
    }

    // Ghi xuống thiết bị thông qua HAL
    int32_t ret = hal_write_sector(sector, buffer);
    if (ret != STATUS_SUCCESS) {
        return STATUS_WRITE_FAILED;
    }

    #if FAT_ENABLE_CACHE
    // Cập nhật cache
    uint32_t cache_index = sector % FAT_CACHE_SIZE;
    fat_ctx.cache[cache_index].sector = sector;
    memcpy(fat_ctx.cache[cache_index].data, buffer, FAT_SECTOR_SIZE);
    fat_ctx.cache[cache_index].valid = true;
    fat_ctx.cache[cache_index].dirty = false;
    #endif

    return STATUS_SUCCESS;
}

int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster)
{
    if (!next_cluster) {
        return STATUS_INVALID_PARAMETER;
    }

    // Tính vị trí entry trong FAT
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    
    switch (fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        default:
            return STATUS_INVALID_PARAMETER;
    }

    // Đọc sector chứa entry
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    int32_t ret = fat_read_sector(fat_sector, sector_buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    // Đọc giá trị entry
    uint32_t entry_value;
    
    switch (fat_type) {
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
            return STATUS_INVALID_PARAMETER;
    }

    *next_cluster = entry_value;
    return STATUS_SUCCESS;
}

int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster)
{
    // Tính vị trí entry trong FAT
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    
    switch (fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = boot_sector.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;
            break;
            
        default:
            return STATUS_INVALID_PARAMETER;
    }

    // Đọc sector chứa entry
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    int32_t ret = fat_read_sector(fat_sector, sector_buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    // Ghi giá trị entry
    switch (fat_type) {
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
            return STATUS_INVALID_PARAMETER;
    }

    // Ghi sector đã cập nhật
    ret = fat_write_sector(fat_sector, sector_buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    return STATUS_SUCCESS;
}

static uint32_t get_fat_size(const fat_boot_sector_t *boot_sector) {
    if (boot_sector->fat_size_16) {
        return boot_sector->fat_size_16;
    }
    /* For FAT32, calculate FAT size based on total sectors */
    uint32_t total_sectors = boot_sector->total_sectors_32;
    uint32_t root_dir_sectors = ((boot_sector->root_entries * sizeof(fat_dir_entry_t)) + 
                               (boot_sector->bytes_per_sector - 1)) / boot_sector->bytes_per_sector;
    uint32_t data_sectors = total_sectors - 
                          (boot_sector->reserved_sectors + root_dir_sectors);
    uint32_t total_clusters = data_sectors / boot_sector->sectors_per_cluster;
    
    /* Calculate FAT size in sectors */
    uint32_t fat_size = (total_clusters * 4 + boot_sector->bytes_per_sector - 1) / 
                       boot_sector->bytes_per_sector;
    return fat_size;
}

static void calculate_layout(const fat_boot_sector_t *boot_sector) {
    /* Tính các thông số cơ bản */
    uint32_t sector_size = boot_sector->bytes_per_sector;
    fat_ctx.fat_size = get_fat_size(boot_sector);
    
    /* Tính vị trí các vùng */
    fat_ctx.first_fat_sector = boot_sector->reserved_sectors;
    fat_ctx.fat_start = boot_sector->reserved_sectors;
    
    /* Tính số sector cho thư mục gốc */
    uint32_t root_dir_sectors = ((boot_sector->root_entries * sizeof(fat_dir_entry_t)) + 
                               (sector_size - 1)) / sector_size;
    
    /* Tính tổng số sector */
    uint32_t total_sectors = boot_sector->total_sectors_16 ? 
                            boot_sector->total_sectors_16 : 
                            boot_sector->total_sectors_32;
                            
    /* Tính số sector dữ liệu */
    uint32_t data_sectors = total_sectors - 
                          (boot_sector->reserved_sectors + 
                           (boot_sector->num_fats * fat_ctx.fat_size) + 
                           root_dir_sectors);
    
    /* Xác định loại FAT dựa trên số cluster */
    uint32_t total_clusters = data_sectors / boot_sector->sectors_per_cluster;
    
    if (total_clusters < 4085) {
        fat_ctx.config.fat_type = FAT_TYPE_12;
        fat_type = FAT_TYPE_12;
    } else if (total_clusters < 65525) {
        fat_ctx.config.fat_type = FAT_TYPE_16;
        fat_type = FAT_TYPE_16;
    } else {
        fat_ctx.config.fat_type = FAT_TYPE_32;
        fat_type = FAT_TYPE_32;
        /* For FAT32, root cluster is stored in the first directory entry */
        fat_ctx.root_cluster = 2; /* Default to cluster 2 for FAT32 */
    }
    
    /* Cập nhật cấu hình */
    fat_ctx.config.sectors_per_cluster = boot_sector->sectors_per_cluster;
    fat_ctx.config.reserved_sectors = boot_sector->reserved_sectors;
    fat_ctx.config.root_dir_sectors = root_dir_sectors;
    fat_ctx.config.total_clusters = total_clusters;
    
    /* Tính sector đầu tiên của vùng dữ liệu */
    fat_ctx.config.first_data_sector = fat_ctx.first_fat_sector +
                                (boot_sector->num_fats * fat_ctx.fat_size) +
                                root_dir_sectors;
}

int32_t fat_deinit(void)
{
    /* Flush cache */
    for (int i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_ctx.cache[i].valid && fat_ctx.cache[i].dirty) {
            if (write_sector(fat_ctx.cache[i].sector, fat_ctx.cache[i].data) != STATUS_SUCCESS) {
                return STATUS_WRITE_FAILED;
            }
        }
    }

    /* Reset context */
    memset(&fat_ctx, 0, sizeof(fat_context_t));
    return STATUS_SUCCESS;
}

static uint32_t fat_cluster_to_sector(uint32_t cluster) {
    if (cluster < 2) {
        return 0;
    }
    /* Công thức: sector = (cluster - 2) * sectors_per_cluster + first_data_sector */
    return ((cluster - 2) * boot_sector.sectors_per_cluster) + fat_ctx.config.first_data_sector;
}

static uint16_t fat_get_time(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    uint16_t time = 0;
    time |= (tm->tm_sec / 2) & 0x1F;     /* Seconds/2 (0-29) */
    time |= (tm->tm_min & 0x3F) << 5;    /* Minutes (0-59) */
    time |= (tm->tm_hour & 0x1F) << 11;  /* Hours (0-23) */
    
    return time;
}

static uint16_t fat_get_date(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    uint16_t date = 0;
    date |= (tm->tm_mday & 0x1F);        /* Day (1-31) */
    date |= ((tm->tm_mon + 1) & 0x0F) << 5;  /* Month (1-12) */
    date |= ((tm->tm_year - 80) & 0x7F) << 9;  /* Year (0-127 + 1980) */
    
    return date;
}

static void fat_get_name(const fat_dir_entry_t *entry, char *name) {
    memcpy(name, entry->name, FAT_DIR_NAME_LEN);
    name[FAT_DIR_NAME_LEN] = '\0';
}

int32_t fat_find_file(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Khởi tạo entry */
    memset(entry, 0, sizeof(fat_dir_entry_t));

    /* Bắt đầu từ cluster gốc */
    uint32_t current_cluster = fat_ctx.root_cluster;
    uint32_t offset = 0;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    /* Đọc sector đầu tiên */
    if (read_sector(fat_cluster_to_sector(current_cluster), sector_buffer) != STATUS_SUCCESS) {
        return STATUS_READ_FAILED;
    }

    /* Tìm entry trong thư mục */
    while (offset < FAT_SECTOR_SIZE) {
        fat_dir_entry_t *current_entry = (fat_dir_entry_t *)&sector_buffer[offset];
        
        /* Kiểm tra entry có hợp lệ không */
        if (current_entry->name[0] == FAT_DIR_EMPTY) {
            break;  /* Hết thư mục */
        }
        
        if (current_entry->name[0] == FAT_DIR_DELETED) {
            offset += FAT_DIR_ENTRY_SIZE;
            continue;
        }

        /* So sánh tên file */
        char entry_name[13];
        fat_get_name(current_entry, entry_name);
        if (strcmp(entry_name, path) == 0) {
            memcpy(entry, current_entry, sizeof(fat_dir_entry_t));
            return STATUS_SUCCESS;
        }

        offset += FAT_DIR_ENTRY_SIZE;
    }

    return STATUS_NOT_FOUND;
}

int32_t fat_create_file(const char *path, fat_dir_entry_t *entry)
{
    if (path == NULL || entry == NULL) {
        return STATUS_INVALID;
    }

    // ... function implementation ...
    return STATUS_SUCCESS;
}

int32_t fat_write_dir_entry(const fat_dir_entry_t *entry)
{
    if (entry == NULL) {
        return STATUS_INVALID;
    }

    // ... function implementation ...
    return STATUS_SUCCESS;
}

uint32_t fat_alloc_cluster(void)
{
    // ... function implementation ...
    return 0;
}

int32_t fat_free_cluster(uint32_t cluster)
{
    if (cluster < 2 || cluster >= fat_ctx.config.total_clusters) {
        return STATUS_INVALID;
    }

    // ... function implementation ...
    return STATUS_SUCCESS;
}

static uint8_t fat_calculate_short_name_checksum(const char *short_name) {
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    }
    return sum;
}

/*********************************************************************
 * UUID: 4b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
