/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm private của module FAT Driver.
 *   Các hàm này chỉ được sử dụng trong nội bộ module.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#define _GNU_SOURCE  /* For strdup */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "../common/common_types.h"
#include "fat_driver_types.h"
#include "fat_driver_private.h"
#include "../hal/hal.h"
#include "fat_driver.h"

/* Global variables */
extern fat_context_t fat_ctx;
extern fat_boot_sector_t boot_sector;
extern uint32_t fat_type;

/*********************************************************************
 * Private Variables
 *********************************************************************/

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/
static uint32_t fat_cluster_to_sector(uint32_t cluster);
static uint16_t fat_get_time(void);
static uint16_t fat_get_date(void);
static int32_t fat_read_sector(uint32_t sector, uint8_t *buffer);
static int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer);
static int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster);
static int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster);
static int32_t fat_find_free_cluster(uint32_t *cluster);
static int32_t fat_convert_to_short_name(const char *name, char *short_name);
static uint8_t fat_calculate_short_name_checksum(const char *short_name);

/* Implementation function prototypes */
static int32_t fat_find_file_impl(const char *path, fat_dir_entry_t *entry);
static int32_t fat_create_file_impl(const char *path, fat_dir_entry_t *entry);
static int32_t fat_write_dir_entry_impl(const fat_dir_entry_t *entry);
static uint32_t fat_alloc_cluster_impl(void);
static int32_t fat_free_cluster_impl(uint32_t cluster);

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
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

static int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
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

static int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster)
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
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    int32_t ret = fat_read_sector(fat_sector, sector_buffer);
    if (ret != STATUS_SUCCESS) {
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
            return STATUS_INVALID_PARAMETER;
    }

    *next_cluster = entry_value;
    return STATUS_SUCCESS;
}

static int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster)
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
    uint8_t sector_buffer[FAT_SECTOR_SIZE];
    int32_t ret = fat_read_sector(fat_sector, sector_buffer);
    if (ret != STATUS_SUCCESS) {
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
            return STATUS_INVALID_PARAMETER;
    }

    // Ghi sector đã cập nhật
    ret = fat_write_sector(fat_sector, sector_buffer);
    if (ret != STATUS_SUCCESS) {
        return ret;
    }

    return STATUS_SUCCESS;
}

static int32_t fat_find_free_cluster(uint32_t *cluster)
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

static int32_t fat_convert_to_short_name(const char *name, char *short_name)
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

static uint8_t fat_calculate_short_name_checksum(const char *short_name)
{
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    }
    return sum;
}

static int32_t fat_find_file_impl(const char *path, fat_dir_entry_t *entry)
{
    if (path == NULL || entry == NULL) {
        return STATUS_INVALID;
    }

    char *path_copy = strdup(path);
    if (!path_copy) {
        return STATUS_NO_MEMORY;
    }

    // Bắt đầu từ thư mục gốc
    uint32_t current_sector = fat_ctx.config.reserved_sectors;
    uint32_t current_cluster = 0;
    uint32_t sector_offset = 0;
    bool found = false;
    
    // Phân tích đường dẫn
    char *token = strtok(path_copy, "/");
    while (token != NULL) {
        found = false;
        
        // Đọc các entry trong thư mục hiện tại
        while (sector_offset < fat_ctx.config.root_dir_sectors) {
            fat_dir_entry_t dir_entry[FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t)];
            
            if (fat_read_sector(current_sector + sector_offset, (uint8_t *)dir_entry) != STATUS_SUCCESS) {
                free(path_copy);
                return STATUS_READ_FAILED;
            }
            
            // Tìm file/thư mục
            for (uint32_t i = 0; i < FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t); i++) {
                if (dir_entry[i].name[0] == FAT_DIR_EMPTY) {
                    break;
                }
                
                if (dir_entry[i].name[0] == FAT_DIR_DELETED) {
                    continue;
                }
                
                char short_name[12] = {0};
                char entry_name[13] = {0};
                
                if (fat_convert_to_short_name(token, short_name) != STATUS_SUCCESS) {
                    free(path_copy);
                    return STATUS_INVALID_NAME;
                }
                
                memcpy(entry_name, dir_entry[i].name, 11);
                
                if (memcmp(short_name, entry_name, 11) == 0) {
                    // Tìm thấy file/thư mục
                    found = true;
                    memcpy(entry, &dir_entry[i], sizeof(fat_dir_entry_t));
                    
                    // Cập nhật vị trí hiện tại
                    current_cluster = (dir_entry[i].first_cluster_hi << 16) | dir_entry[i].first_cluster_lo;
                    current_sector = fat_cluster_to_sector(current_cluster);
                    
                    break;
                }
            }
            
            if (found) {
                break;
            }
            
            sector_offset++;
        }
        
        if (!found) {
            free(path_copy);
            return STATUS_NOT_FOUND;
        }
        
        token = strtok(NULL, "/");
    }
    
    free(path_copy);
    return STATUS_SUCCESS;
}

static int32_t fat_create_file_impl(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return STATUS_INVALID_PARAMETER;
    }

    // Kiểm tra xem file đã tồn tại chưa
    fat_dir_entry_t existing_entry;
    memset(&existing_entry, 0, sizeof(fat_dir_entry_t)); // Khởi tạo existing_entry
    int32_t result = fat_find_file_impl(path, &existing_entry);
    
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
                
                dir_entry[i].attributes = FAT_ATTR_ARCHIVE;
                dir_entry[i].creation_time = fat_get_time();
                dir_entry[i].creation_date = fat_get_date();
                dir_entry[i].last_access_date = fat_get_date();
                dir_entry[i].last_write_time = fat_get_time();
                dir_entry[i].last_write_date = fat_get_date();
                
                // Cấp phát cluster đầu tiên
                uint32_t first_cluster = fat_alloc_cluster_impl();
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

static int32_t fat_write_dir_entry_impl(const fat_dir_entry_t *entry)
{
    if (!entry) {
        return STATUS_INVALID_PARAMETER;
    }

    // Tìm vị trí entry trong thư mục
    uint32_t current_sector = fat_ctx.config.reserved_sectors;
    uint32_t sector_offset = 0;

    // TODO: Implement directory entry writing
    (void)current_sector; // Unused parameter
    (void)sector_offset; // Unused parameter

    return STATUS_ERROR;
}

static uint32_t fat_alloc_cluster_impl(void) {
    uint32_t cluster;
    
    if (fat_find_free_cluster(&cluster) != STATUS_SUCCESS) {
        return 0;
    }
    
    if (fat_write_fat_entry(cluster, FAT_EOC(fat_ctx.config.fat_type)) != STATUS_SUCCESS) {
        return 0;
    }
    
    return cluster;
}

static int32_t fat_free_cluster_impl(uint32_t cluster) {
    if (cluster < 2 || cluster >= fat_ctx.config.total_clusters) {
        return STATUS_INVALID_PARAMETER;
    }
    
    uint32_t current_cluster = cluster;
    uint32_t next_cluster;
    
    do {
        if (fat_read_fat_entry(current_cluster, &next_cluster) != STATUS_SUCCESS) {
            return STATUS_READ_FAILED;
        }
        
        // Đánh dấu cluster là trống
        if (fat_write_fat_entry(current_cluster, FAT_FREE_CLUSTER) != STATUS_SUCCESS) {
            return STATUS_WRITE_FAILED;
        }
        
        current_cluster = next_cluster;
    } while (current_cluster != FAT_EOC(fat_ctx.config.fat_type));
    
    return STATUS_SUCCESS;
}

/**
 * @brief Lấy tên file từ entry
 */
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

/**
 * @brief Chuyển cluster thành sector
 */
uint32_t fat_cluster_to_sector(uint32_t cluster) {
    if (cluster < 2) {
        return 0;
    }
    return ((cluster - 2) * boot_sector.sectors_per_cluster) + fat_ctx.config.first_data_sector;
}

/**
 * @brief Lấy thời gian hiện tại
 */
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

/**
 * @brief Lấy ngày hiện tại
 */
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

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
