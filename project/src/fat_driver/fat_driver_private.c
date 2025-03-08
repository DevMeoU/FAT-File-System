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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>
#include "fat_driver_private.h"
#include "../ip_driver/ip_driver.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
fat_context_t fat_context;
fat_cache_entry_t fat_cache[FAT_CACHE_SIZE] = {0};

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

int32_t fat_read_sector(uint32_t sector, uint8_t *buffer)
{
    if (buffer == NULL) {
        return FAT_INVALID;
    }

    /* Search in cache */
    uint32_t min_access = UINT32_MAX;
    uint32_t min_index = 0;
    for (uint32_t i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].sector == sector) {
            /* Cache hit */
            memcpy(buffer, fat_cache[i].data, FAT_SECTOR_SIZE);
            fat_cache[i].access_count++;
            return FAT_SUCCESS;
        }
        if (fat_cache[i].access_count < min_access) {
            min_access = fat_cache[i].access_count;
            min_index = i;
        }
    }

    /* Cache miss - read from device */
    if (ip_read_sector(sector, buffer) != IP_SUCCESS) {
        return FAT_ERROR;
    }

    /* Update cache */
    fat_cache[min_index].sector = sector;
    memcpy(fat_cache[min_index].data, buffer, FAT_SECTOR_SIZE);
    fat_cache[min_index].access_count = 1;

    return FAT_SUCCESS;
}

int32_t fat_write_sector(uint32_t sector, const uint8_t *buffer)
{
    if (buffer == NULL) {
        return FAT_INVALID;
    }

    /* Search in cache */
    uint32_t min_access = UINT32_MAX;
    uint32_t min_index = 0;
    for (uint32_t i = 0; i < FAT_CACHE_SIZE; i++) {
        if (fat_cache[i].sector == sector) {
            /* Cache hit */
            memcpy(fat_cache[i].data, buffer, FAT_SECTOR_SIZE);
            fat_cache[i].access_count++;
            fat_cache[i].dirty = true;
            return FAT_SUCCESS;
        }
        if (fat_cache[i].access_count < min_access) {
            min_access = fat_cache[i].access_count;
            min_index = i;
        }
    }

    /* Write to device */
    if (ip_write_sector(sector, buffer) != IP_SUCCESS) {
        return FAT_ERROR;
    }

    /* Update cache */
    fat_cache[min_index].sector = sector;
    memcpy(fat_cache[min_index].data, buffer, FAT_SECTOR_SIZE);
    fat_cache[min_index].access_count = 1;
    fat_cache[min_index].dirty = false;

    return FAT_SUCCESS;
}

int32_t fat_read_fat_entry(uint32_t cluster, uint32_t *next_cluster)
{
    /* Check parameters */
    if (next_cluster == NULL || cluster >= fat_context.config.total_clusters) {
        return FAT_INVALID;
    }

    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    switch (fat_context.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            if (ent_offset == (FAT_SECTOR_SIZE - 1)) {
                /* Entry spans two sectors */
                uint8_t next_sector_buffer[FAT_SECTOR_SIZE];
                if (fat_read_sector(fat_sector + 1, next_sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }

                if (cluster & 0x1) {
                    *next_cluster = (sector_buffer[ent_offset] >> 4) | (next_sector_buffer[0] << 4);
                } else {
                    *next_cluster = sector_buffer[ent_offset] | ((next_sector_buffer[0] & 0x0F) << 8);
                }
            } else {
                if (cluster & 0x1) {
                    *next_cluster = (sector_buffer[ent_offset] >> 4) | (sector_buffer[ent_offset + 1] << 4);
                } else {
                    *next_cluster = sector_buffer[ent_offset] | ((sector_buffer[ent_offset + 1] & 0x0F) << 8);
                }
            }
            *next_cluster &= FAT12_MASK;
            break;

        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *next_cluster = *(uint16_t *)&sector_buffer[ent_offset];
            break;

        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *next_cluster = *(uint32_t *)&sector_buffer[ent_offset] & FAT32_MASK;
            break;

        default:
            return FAT_ERROR;
    }

    return FAT_SUCCESS;
}

int32_t fat_write_fat_entry(uint32_t cluster, uint32_t next_cluster)
{
    /* Check parameters */
    if (cluster >= fat_context.config.total_clusters) {
        return FAT_INVALID;
    }

    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;
    uint8_t sector_buffer[FAT_SECTOR_SIZE];

    switch (fat_context.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            if (ent_offset == (FAT_SECTOR_SIZE - 1)) {
                /* Entry spans two sectors */
                uint8_t next_sector_buffer[FAT_SECTOR_SIZE];
                if (fat_read_sector(fat_sector + 1, next_sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }

                if (cluster & 0x1) {
                    sector_buffer[ent_offset] = (sector_buffer[ent_offset] & 0x0F) | ((next_cluster & 0x0F) << 4);
                    next_sector_buffer[0] = (next_cluster >> 4) & 0xFF;
                } else {
                    sector_buffer[ent_offset] = next_cluster & 0xFF;
                    next_sector_buffer[0] = (next_sector_buffer[0] & 0xF0) | ((next_cluster >> 8) & 0x0F);
                }

                if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }
                if (fat_write_sector(fat_sector + 1, next_sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }
            } else {
                if (cluster & 0x1) {
                    sector_buffer[ent_offset] = (sector_buffer[ent_offset] & 0x0F) | ((next_cluster & 0x0F) << 4);
                    sector_buffer[ent_offset + 1] = (next_cluster >> 4) & 0xFF;
                } else {
                    sector_buffer[ent_offset] = next_cluster & 0xFF;
                    sector_buffer[ent_offset + 1] = (sector_buffer[ent_offset + 1] & 0xF0) | ((next_cluster >> 8) & 0x0F);
                }

                if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                    return FAT_ERROR;
                }
            }
            break;

        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *(uint16_t *)&sector_buffer[ent_offset] = (uint16_t)next_cluster;

            if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
            break;

        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            fat_sector = fat_context.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
            ent_offset = fat_offset % FAT_SECTOR_SIZE;

            if (fat_read_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            *(uint32_t *)&sector_buffer[ent_offset] = (*(uint32_t *)&sector_buffer[ent_offset] & ~FAT32_MASK) | (next_cluster & FAT32_MASK);

            if (fat_write_sector(fat_sector, sector_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }
            break;

        default:
            return FAT_ERROR;
    }

    return FAT_SUCCESS;
}

int32_t fat_find_free_cluster(uint32_t *cluster)
{
    /* Check parameters */
    if (cluster == NULL) {
        return FAT_INVALID;
    }

    uint32_t current_cluster = 2;  /* First valid cluster */
    uint32_t next_cluster;

    while (current_cluster < fat_context.config.total_clusters) {
        if (fat_read_fat_entry(current_cluster, &next_cluster) != FAT_SUCCESS) {
            return FAT_ERROR;
        }

        if (next_cluster == FAT_FREE_CLUSTER) {
            *cluster = current_cluster;
            return FAT_SUCCESS;
        }

        current_cluster++;
    }

    return FAT_DISK_FULL;
}

int32_t fat_convert_to_short_name(const char *name, char *short_name)
{
    if (name == NULL || short_name == NULL) {
        return FAT_INVALID;
    }

    /* Find extension */
    const char *ext = strrchr(name, '.');
    size_t name_len = (ext != NULL) ? (size_t)(ext - name) : strlen(name);

    /* Check name length */
    if (name_len > FAT_DIR_NAME_LEN) {
        return FAT_INVALID_NAME;
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

    return FAT_SUCCESS;
}

uint8_t fat_calculate_short_name_checksum(const char *short_name)
{
    uint8_t sum = 0;
    for (int i = 0; i < 11; i++) {
        sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + short_name[i];
    }
    return sum;
}

int32_t fat_find_file(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    char *path_copy = strdup(path);
    if (!path_copy) {
        return FAT_ERROR_NO_MEMORY;
    }

    // Bắt đầu từ thư mục gốc
    uint32_t current_sector = fat_context.config.reserved_sectors;
    uint32_t current_cluster = 0;
    uint32_t sector_offset = 0;
    fat_dir_entry_t dir_entry[FAT_DIR_ENTRY_SIZE];

    char *token = strtok(path_copy, "/");
    while (token) {
        bool found = false;
        
        // Đọc các entry trong thư mục hiện tại
        while (sector_offset < fat_context.config.root_dir_sectors) {
            if (fat_read_sector(current_sector + sector_offset, (uint8_t *)dir_entry) != FAT_SUCCESS) {
                free(path_copy);
                return FAT_ERROR_READ_FAILED;
            }

            // Tìm entry phù hợp
            for (int j = 0; j < FAT_DIR_ENTRY_SIZE; j++) {
                if (dir_entry[j].name[0] == FAT_DIR_EMPTY) {
                    break;
                }
                if (dir_entry[j].name[0] == FAT_DIR_DELETED) {
                    continue;
                }

                char name[FAT_DIR_NAME_LEN + 1];
                fat_get_name(&dir_entry[j], name);

                if (strcmp(name, token) == 0) {
                    memcpy(entry, &dir_entry[j], sizeof(fat_dir_entry_t));
                    found = true;
                    
                    // Cập nhật vị trí cho thư mục tiếp theo
                    current_cluster = ((uint32_t)entry->first_cluster_hi << 16) | entry->first_cluster_lo;
                    if (current_cluster != 0) {
                        current_sector = fat_cluster_to_sector(current_cluster);
                        sector_offset = 0;
                    }
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
            return FAT_ERROR_NOT_FOUND;
        }

        token = strtok(NULL, "/");
    }

    free(path_copy);
    return FAT_SUCCESS;
}

int32_t fat_create_file(const char *path, fat_dir_entry_t *entry)
{
    if (!path || !entry) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    // Tìm thư mục cha
    char *path_copy = strdup(path);
    char *last_slash = strrchr(path_copy, '/');
    char *parent_path = NULL;
    char *file_name = NULL;

    if (last_slash) {
        *last_slash = '\0';
        parent_path = path_copy;
        file_name = last_slash + 1;
    } else {
        parent_path = ".";
        file_name = path_copy;
    }

    fat_dir_entry_t parent_entry;
    int32_t result = fat_find_file(parent_path, &parent_entry);
    if (result != FAT_SUCCESS) {
        free(path_copy);
        return result;
    }

    // Tìm entry trống trong thư mục cha
    uint32_t parent_cluster = ((uint32_t)parent_entry.first_cluster_hi << 16) | parent_entry.first_cluster_lo;
    uint32_t current_sector = fat_cluster_to_sector(parent_cluster);
    fat_dir_entry_t dir_entry[FAT_DIR_ENTRY_SIZE];

    while (1) {
        if (fat_read_sector(current_sector, (uint8_t *)dir_entry) != FAT_SUCCESS) {
            free(path_copy);
            return FAT_ERROR_READ_FAILED;
        }

        // Tìm entry trống
        for (int i = 0; i < FAT_DIR_ENTRY_SIZE; i++) {
            if (dir_entry[i].name[0] == FAT_DIR_EMPTY || 
                dir_entry[i].name[0] == FAT_DIR_DELETED) {
                
                // Khởi tạo entry mới
                memset(&dir_entry[i], 0, sizeof(fat_dir_entry_t));
                strncpy((char *)dir_entry[i].name, file_name, FAT_DIR_NAME_LEN);
                dir_entry[i].attributes = FAT_ATTR_ARCHIVE;
                dir_entry[i].creation_time = fat_get_time();
                dir_entry[i].creation_date = fat_get_date();
                dir_entry[i].last_access_date = dir_entry[i].creation_date;
                dir_entry[i].last_write_time = dir_entry[i].creation_time;
                dir_entry[i].last_write_date = dir_entry[i].creation_date;

                // Cấp phát cluster đầu tiên
                uint32_t first_cluster;
                if (fat_find_free_cluster(&first_cluster) != FAT_SUCCESS) {
                    free(path_copy);
                    return FAT_ERROR_NO_SPACE;
                }

                dir_entry[i].first_cluster_hi = (uint16_t)(first_cluster >> 16);
                dir_entry[i].first_cluster_lo = (uint16_t)(first_cluster & 0xFFFF);
                if (fat_write_fat_entry(first_cluster, FAT_EOC(fat_context.config.fat_type)) != FAT_SUCCESS) {
                    free(path_copy);
                    return FAT_ERROR_WRITE_FAILED;
                }

                // Ghi entry vào sector
                if (fat_write_sector(current_sector, (uint8_t *)dir_entry) != FAT_SUCCESS) {
                    free(path_copy);
                    return FAT_ERROR_WRITE_FAILED;
                }

                memcpy(entry, &dir_entry[i], sizeof(fat_dir_entry_t));
                free(path_copy);
                return FAT_SUCCESS;
            }
        }

        // Chuyển sang sector tiếp theo
        uint32_t next_cluster;
        if (fat_read_fat_entry(parent_cluster, &next_cluster) != FAT_SUCCESS) {
            free(path_copy);
            return FAT_ERROR_READ_FAILED;
        }

        if (next_cluster >= FAT_EOC(fat_context.config.fat_type)) {
            // Cấp phát cluster mới cho thư mục
            uint32_t new_cluster;
            if (fat_find_free_cluster(&new_cluster) != FAT_SUCCESS) {
                free(path_copy);
                return FAT_ERROR_NO_SPACE;
            }

            if (fat_write_fat_entry(parent_cluster, new_cluster) != FAT_SUCCESS ||
                fat_write_fat_entry(new_cluster, FAT_EOC(fat_context.config.fat_type)) != FAT_SUCCESS) {
                free(path_copy);
                return FAT_ERROR_WRITE_FAILED;
            }

            parent_cluster = new_cluster;
        } else {
            parent_cluster = next_cluster;
        }

        current_sector = fat_cluster_to_sector(parent_cluster);
    }
}

int32_t fat_write_dir_entry(const fat_dir_entry_t *entry)
{
    if (!entry) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    // Tìm vị trí entry trong thư mục
    uint32_t current_sector = fat_context.config.reserved_sectors;
    uint32_t sector_offset = 0;
    fat_dir_entry_t dir_entry[FAT_DIR_ENTRY_SIZE];

    while (sector_offset < fat_context.config.root_dir_sectors) {
        if (fat_read_sector(current_sector + sector_offset, (uint8_t *)dir_entry) != FAT_SUCCESS) {
            return FAT_ERROR_READ_FAILED;
        }

        for (int i = 0; i < FAT_DIR_ENTRY_SIZE; i++) {
            if (memcmp(&dir_entry[i], entry, sizeof(fat_dir_entry_t)) == 0) {
                // Cập nhật entry
                memcpy(&dir_entry[i], entry, sizeof(fat_dir_entry_t));
                
                // Ghi lại sector
                if (fat_write_sector(current_sector + sector_offset, (uint8_t *)dir_entry) != FAT_SUCCESS) {
                    return FAT_ERROR_WRITE_FAILED;
                }
                
                return FAT_SUCCESS;
            }
        }

        sector_offset++;
    }

    return FAT_ERROR_NOT_FOUND;
}

uint32_t fat_alloc_cluster(void) {
    uint32_t cluster;
    if (fat_find_free_cluster(&cluster) != FAT_SUCCESS) {
        return 0;
    }
    
    if (fat_write_fat_entry(cluster, FAT_EOC(fat_context.config.fat_type)) != FAT_SUCCESS) {
        return 0;
    }
    
    return cluster;
}

int32_t fat_free_cluster(uint32_t cluster) {
    if (cluster < 2 || cluster >= fat_context.config.total_clusters) {
        return FAT_ERROR_INVALID_PARAMETER;
    }

    uint32_t current_cluster = cluster;
    uint32_t next_cluster;

    do {
        if (fat_read_fat_entry(current_cluster, &next_cluster) != FAT_SUCCESS) {
            return FAT_ERROR_READ_FAILED;
        }

        if (fat_write_fat_entry(current_cluster, FAT_FREE_CLUSTER) != FAT_SUCCESS) {
            return FAT_ERROR_WRITE_FAILED;
        }

        current_cluster = next_cluster;
    } while (current_cluster != FAT_EOC(fat_context.config.fat_type));

    return FAT_SUCCESS;
}

/* Định nghĩa FAT_EOC dựa trên loại FAT */
#define FAT_EOC(type) ((type) == FAT_TYPE_FAT12 ? FAT12_EOC : \
                       (type) == FAT_TYPE_FAT16 ? FAT16_EOC : \
                       FAT32_EOC)

/**
 * @brief Lấy tên file từ entry
 */
void fat_get_name(const fat_dir_entry_t *entry, char *name) {
    memcpy(name, entry->name, FAT_DIR_NAME_LEN);
    name[FAT_DIR_NAME_LEN] = '\0';
}

/**
 * @brief Chuyển cluster thành sector
 */
uint32_t fat_cluster_to_sector(uint32_t cluster) {
    return fat_context.config.first_data_sector + 
           ((cluster - 2) * fat_context.config.sectors_per_cluster);
}

/**
 * @brief Lấy thời gian hiện tại
 */
uint16_t fat_get_time(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec >> 1);
}

/**
 * @brief Lấy ngày hiện tại
 */
uint16_t fat_get_date(void) {
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    return ((tm->tm_year - 80) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;
}

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 