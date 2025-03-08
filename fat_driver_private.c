#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "common_types.h"
#include "fat_driver.h"
#include "fat_driver_private.h"
#include "storage_driver.h"

// Định nghĩa biến fat_ctx
fat_context_t fat_ctx;

/*
 * Đọc một sector từ thiết bị lưu trữ
 */
int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID_PARAMETER;
    }

    #if FAT_ENABLE_CACHE
    // Kiểm tra cache
    uint32_t cache_index = sector % FAT_CACHE_SIZE;
    if (fat_ctx.cache[cache_index].valid && fat_ctx.cache[cache_index].sector == sector) {
        memcpy(buffer, fat_ctx.cache[cache_index].data, FAT_SECTOR_SIZE);
        return STATUS_SUCCESS;
    }
    #endif

    // Đọc từ thiết bị thông qua storage driver
    int32_t ret = storage_read_sector(sector, buffer);
    if (ret != STATUS_SUCCESS) {
        return STATUS_READ_FAILED;
    }

    #if FAT_ENABLE_CACHE
    // Cập nhật cache
    fat_ctx.cache[cache_index].sector = sector;
    memcpy(fat_ctx.cache[cache_index].data, buffer, FAT_SECTOR_SIZE);
    fat_ctx.cache[cache_index].valid = true;
    fat_ctx.cache[cache_index].dirty = false;
    #endif

    return STATUS_SUCCESS;
}

/*
 * Ghi một sector xuống thiết bị lưu trữ
 */
int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID_PARAMETER;
    }

    // Ghi xuống thiết bị thông qua storage driver
    int32_t ret = storage_write_sector(sector, buffer);
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

int32_t fat_find_free_cluster(uint32_t *cluster)
{
    if (!cluster) {
        return STATUS_INVALID_PARAMETER;
    }

    uint32_t current_cluster = 2;  /* First valid cluster */
    uint32_t next_cluster;

    while (current_cluster < fat_ctx.config.total_clusters) {
        if (fat_read_fat_entry(current_cluster, &next_cluster) != STATUS_SUCCESS) {
            return STATUS_ERROR;
        }

        if (next_cluster == FAT_FREE_CLUSTER) {
            *cluster = current_cluster;
            return STATUS_SUCCESS;
        }

        current_cluster++;
    }

    return STATUS_DISK_FULL;
}

int32_t fat_convert_to_short_name(const char *name, char *short_name)
{
    if (name == NULL || short_name == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Find extension */
    const char *ext = strrchr(name, '.');
    size_t name_len = (ext != NULL) ? (size_t)(ext - name) : strlen(name);

    /* Check name length */
    if (name_len > FAT_DIR_NAME_LEN) {
        return STATUS_INVALID_NAME;
    }

    /* Clear short name buffer */
    memset(short_name, ' ', 11);

    /* Copy name */
    for (size_t i = 0; i < name_len && i < FAT_DIR_NAME_LEN; i++) {
        short_name[i] = toupper(name[i]);
    }

    /* Copy extension */
    if (ext != NULL) {
        for (size_t i = 0; i < strlen(ext + 1) && i < FAT_DIR_EXT_LEN; i++) {
            short_name[FAT_DIR_NAME_LEN + i] = toupper(ext[i + 1]);
        }
    }

    return STATUS_SUCCESS;
}

int32_t fat_find_file(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return STATUS_INVALID_PARAMETER;
    }

    // ... existing code ...

    if (!found) {
        free(path_copy);
        return STATUS_NOT_FOUND;
    }

    free(path_copy);
    return STATUS_SUCCESS;
}

int32_t fat_write_dir_entry(const fat_dir_entry_t *entry)
{
    if (!entry) {
        return STATUS_INVALID_PARAMETER;
    }

    // ... existing code ...

    return STATUS_NOT_FOUND;
}

int32_t fat_free_cluster(uint32_t cluster)
{
    if (cluster < 2 || cluster >= fat_ctx.config.total_clusters) {
        return STATUS_INVALID_PARAMETER;
    }

    // ... existing code ...

    return STATUS_SUCCESS;
}

uint32_t fat_cluster_to_sector(uint32_t cluster)
{
    if (cluster < 2) {
        return 0;
    }
    return ((cluster - 2) * fat_ctx.config.sectors_per_cluster) + fat_ctx.config.first_data_sector;
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
            return STATUS_INVALID_PARAMETER;
    }
    
    // Đọc sector chứa entry
    uint8_t buffer[FAT_SECTOR_SIZE];
    if (fat_read_sector(fat_sector, buffer) != STATUS_SUCCESS) {
        return STATUS_READ_FAILED;
    }
    
    // Đọc giá trị entry
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12: {
            uint16_t value = *((uint16_t*)&buffer[ent_offset]);
            if (cluster & 1) {
                value = value >> 4;
            } else {
                value = value & 0x0FFF;
            }
            *next_cluster = value;
            break;
        }
        
        case FAT_TYPE_16:
            *next_cluster = *((uint16_t*)&buffer[ent_offset]);
            break;
            
        case FAT_TYPE_32:
            *next_cluster = *((uint32_t*)&buffer[ent_offset]) & 0x0FFFFFFF;
            break;
    }
    
    return STATUS_SUCCESS;
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
            return STATUS_INVALID_PARAMETER;
    }
    
    // Đọc sector chứa entry
    uint8_t buffer[FAT_SECTOR_SIZE];
    if (fat_read_sector(fat_sector, buffer) != STATUS_SUCCESS) {
        return STATUS_READ_FAILED;
    }
    
    // Ghi giá trị entry
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12: {
            uint16_t value = *((uint16_t*)&buffer[ent_offset]);
            if (cluster & 1) {
                value = (value & 0x000F) | ((next_cluster & 0x0FFF) << 4);
            } else {
                value = (value & 0xF000) | (next_cluster & 0x0FFF);
            }
            *((uint16_t*)&buffer[ent_offset]) = value;
            break;
        }
        
        case FAT_TYPE_16:
            *((uint16_t*)&buffer[ent_offset]) = (uint16_t)next_cluster;
            break;
            
        case FAT_TYPE_32: {
            uint32_t value = *((uint32_t*)&buffer[ent_offset]);
            value = (value & 0xF0000000) | (next_cluster & 0x0FFFFFFF);
            *((uint32_t*)&buffer[ent_offset]) = value;
            break;
        }
    }
    
    // Ghi sector
    if (fat_write_sector(fat_sector, buffer) != STATUS_SUCCESS) {
        return STATUS_WRITE_FAILED;
    }
    
    return STATUS_SUCCESS;
}

int32_t fat_create_file(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return STATUS_INVALID_PARAMETER;
    }

    // Kiểm tra xem file đã tồn tại chưa
    fat_dir_entry_t existing_entry;
    int32_t result = fat_find_file(path, &existing_entry);
    
    if (result == STATUS_SUCCESS) {
        // File đã tồn tại
        memcpy(entry, &existing_entry, sizeof(fat_dir_entry_t));
        return STATUS_SUCCESS;
    }
    
    // Tìm thư mục cha
    char *path_copy = strdup(path);
    if (!path_copy) {
        return STATUS_NO_MEMORY;
    }
    
    char *file_name = strrchr(path_copy, '/');
    if (file_name) {
        *file_name = '\0';
        file_name++;
    } else {
        file_name = path_copy;
    }
    
    // Tìm thư mục cha để thêm entry mới
    uint32_t current_sector = fat_ctx.config.reserved_sectors;
    uint32_t sector_offset = 0;
    
    while (sector_offset < fat_ctx.config.root_dir_sectors) {
        fat_dir_entry_t dir_entry[FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t)];
        
        if (fat_read_sector(current_sector + sector_offset, (uint8_t *)dir_entry) != STATUS_SUCCESS) {
            free(path_copy);
            return STATUS_READ_FAILED;
        }
        
        // Tìm entry trống
        for (uint32_t i = 0; i < FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t); i++) {
            if (dir_entry[i].name[0] == FAT_DIR_EMPTY || dir_entry[i].name[0] == FAT_DIR_DELETED) {
                // Đã tìm thấy entry trống, tạo file mới
                memset(&dir_entry[i], 0, sizeof(fat_dir_entry_t));
                
                // Chuyển đổi tên file sang định dạng 8.3
                if (fat_convert_to_short_name(file_name, (char *)dir_entry[i].name) != STATUS_SUCCESS) {
                    free(path_copy);
                    return STATUS_NO_SPACE;
                }
                
                dir_entry[i].attribute = FAT_ATTR_ARCHIVE;
                dir_entry[i].create_time = fat_get_time();
                dir_entry[i].create_date = fat_get_date();
                dir_entry[i].last_access_date = fat_get_date();
                dir_entry[i].last_write_time = fat_get_time();
                dir_entry[i].last_write_date = fat_get_date();
                
                // Cấp phát cluster đầu tiên
                uint32_t first_cluster = fat_alloc_cluster();
                if (first_cluster == 0) {
                    free(path_copy);
                    return STATUS_NO_SPACE;
                }
                
                dir_entry[i].first_cluster_hi = (uint16_t)(first_cluster >> 16);
                dir_entry[i].first_cluster_lo = (uint16_t)(first_cluster & 0xFFFF);
                
                // Ghi lại sector chứa entry
                if (fat_write_sector(current_sector + sector_offset, (uint8_t *)dir_entry) != STATUS_SUCCESS) {
                    free(path_copy);
                    return STATUS_WRITE_FAILED;
                }
                
                // Trả về entry vừa tạo
                memcpy(entry, &dir_entry[i], sizeof(fat_dir_entry_t));
                
                free(path_copy);
                return STATUS_SUCCESS;
            }
        }
        
        sector_offset++;
    }
    
    free(path_copy);
    return STATUS_NO_SPACE;
}

#undef FAT_EOC

// Thay thế hàm này bằng đoạn mã sau
void fat_get_name(const fat_dir_entry_t *entry, char *name)
{
    if (!entry || !name) {
        return;
    }
    
    // Copy tên file (8 ký tự)
    memcpy(name, entry->name, FAT_DIR_NAME_LEN);
    
    // Thêm dấu chấm và phần mở rộng nếu có
    if (entry->name[8] != ' ') {
        name[FAT_DIR_NAME_LEN] = '.';
        memcpy(name + FAT_DIR_NAME_LEN + 1, entry->name + 8, FAT_DIR_EXT_LEN);
        name[FAT_DIR_NAME_LEN + 1 + FAT_DIR_EXT_LEN] = '\0';
    } else {
        name[FAT_DIR_NAME_LEN] = '\0';
    }
    
    // Loại bỏ khoảng trắng thừa
    for (int i = strlen(name) - 1; i >= 0 && name[i] == ' '; i--) {
        name[i] = '\0';
    }
}

uint16_t fat_get_time(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    uint16_t time = 0;
    time |= (tm->tm_sec / 2) & 0x1F;     /* Seconds/2 (0-29) */
    time |= (tm->tm_min & 0x3F) << 5;    /* Minutes (0-59) */
    time |= (tm->tm_hour & 0x1F) << 11;  /* Hours (0-23) */
    
    return time;
}

uint16_t fat_get_date(void)
{
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    uint16_t date = 0;
    date |= (tm->tm_mday & 0x1F);        /* Day (1-31) */
    date |= ((tm->tm_mon + 1) & 0x0F) << 5;  /* Month (1-12) */
    date |= ((tm->tm_year - 80) & 0x7F) << 9;  /* Year (0-127 + 1980) */
    
    return date;
}

// ... existing code ... 