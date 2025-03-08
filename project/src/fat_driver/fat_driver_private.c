/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File nguồn riêng của module FAT Driver, cài đặt các hàm private
 *   chỉ sử dụng trong nội bộ module.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fat_driver_private.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/

/* Sector buffer */
static uint8_t sector_buffer[FAT_SECTOR_SIZE];

/* FAT table buffer */
static uint8_t fat_buffer[FAT_SECTOR_SIZE];

/* Directory entry buffer */
static uint8_t dir_buffer[FAT_SECTOR_SIZE];

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

/**
 * @brief Tính cluster tiếp theo từ bảng FAT
 * 
 * @param cluster Cluster hiện tại
 * @param next_cluster Con trỏ đến cluster tiếp theo
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t fat_get_next_cluster(uint32_t cluster, uint32_t *next_cluster)
{
    uint32_t fat_offset;
    uint32_t fat_sector;
    uint32_t ent_offset;

    /* Tính offset trong bảng FAT */
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            fat_offset = cluster + (cluster / 2);
            break;
        case FAT_TYPE_16:
            fat_offset = cluster * 2;
            break;
        case FAT_TYPE_32:
            fat_offset = cluster * 4;
            break;
        default:
            return FAT_INVALID;
    }

    /* Tính sector chứa entry */
    fat_sector = fat_ctx.config.reserved_sectors + (fat_offset / FAT_SECTOR_SIZE);
    ent_offset = fat_offset % FAT_SECTOR_SIZE;

    /* Đọc sector */
    if (fat_read_sector(fat_sector, fat_buffer) != FAT_SUCCESS) {
        return FAT_ERROR;
    }

    /* Đọc giá trị entry */
    switch (fat_ctx.config.fat_type) {
        case FAT_TYPE_12:
            if (cluster & 0x1) {
                *next_cluster = (*(uint16_t *)&fat_buffer[ent_offset] >> 4) & FAT12_MASK;
            } else {
                *next_cluster = *(uint16_t *)&fat_buffer[ent_offset] & FAT12_MASK;
            }
            break;
        case FAT_TYPE_16:
            *next_cluster = *(uint16_t *)&fat_buffer[ent_offset] & FAT16_MASK;
            break;
        case FAT_TYPE_32:
            *next_cluster = *(uint32_t *)&fat_buffer[ent_offset] & FAT32_MASK;
            break;
    }

    return FAT_SUCCESS;
}

/**
 * @brief Tìm entry trong thư mục
 * 
 * @param dir_cluster Cluster của thư mục
 * @param name Tên cần tìm
 * @param entry Con trỏ đến entry tìm được
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t fat_find_directory_entry(uint32_t dir_cluster, const char *name, fat_dir_entry_t *entry)
{
    uint32_t sector;
    uint32_t cluster = dir_cluster;
    char short_name[12];

    /* Chuyển đổi tên sang định dạng 8.3 */
    if (fat_convert_to_short_name(name, short_name) != FAT_SUCCESS) {
        return FAT_INVALID_NAME;
    }

    /* Duyệt qua các cluster của thư mục */
    while (cluster != FAT_INVALID_CLUSTER) {
        /* Tính sector đầu tiên của cluster */
        sector = fat_ctx.config.first_data_sector + 
                 ((cluster - 2) * fat_ctx.config.sectors_per_cluster);

        /* Duyệt qua các sector trong cluster */
        for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
            /* Đọc sector */
            if (fat_read_sector(sector + i, dir_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            /* Duyệt qua các entry trong sector */
            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)dir_buffer;
            for (uint32_t j = 0; j < FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t); j++) {
                /* Kiểm tra entry có hợp lệ không */
                if (dir_entry[j].name[0] == FAT_DIR_EMPTY) {
                    return FAT_NOT_FOUND;
                }
                if (dir_entry[j].name[0] == FAT_DIR_DELETED) {
                    continue;
                }

                /* So sánh tên */
                if (memcmp(dir_entry[j].name, short_name, 11) == 0) {
                    memcpy(entry, &dir_entry[j], sizeof(fat_dir_entry_t));
                    return FAT_SUCCESS;
                }
            }
        }

        /* Lấy cluster tiếp theo */
        if (fat_get_next_cluster(cluster, &cluster) != FAT_SUCCESS) {
            return FAT_ERROR;
        }
    }

    return FAT_NOT_FOUND;
}

/**
 * @brief Tạo entry mới trong thư mục
 * 
 * @param dir_cluster Cluster của thư mục
 * @param name Tên entry
 * @param attributes Thuộc tính entry
 * @param entry Con trỏ đến entry được tạo
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t fat_create_directory_entry(uint32_t dir_cluster, const char *name, 
                                        uint8_t attributes, fat_dir_entry_t *entry)
{
    uint32_t sector;
    uint32_t cluster = dir_cluster;
    char short_name[12];

    /* Chuyển đổi tên sang định dạng 8.3 */
    if (fat_convert_to_short_name(name, short_name) != FAT_SUCCESS) {
        return FAT_INVALID_NAME;
    }

    /* Duyệt qua các cluster của thư mục */
    while (cluster != FAT_INVALID_CLUSTER) {
        /* Tính sector đầu tiên của cluster */
        sector = fat_ctx.config.first_data_sector + 
                 ((cluster - 2) * fat_ctx.config.sectors_per_cluster);

        /* Duyệt qua các sector trong cluster */
        for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
            /* Đọc sector */
            if (fat_read_sector(sector + i, dir_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            /* Duyệt qua các entry trong sector */
            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)dir_buffer;
            for (uint32_t j = 0; j < FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t); j++) {
                /* Tìm entry trống */
                if (dir_entry[j].name[0] == FAT_DIR_EMPTY || 
                    dir_entry[j].name[0] == FAT_DIR_DELETED) {
                    /* Khởi tạo entry mới */
                    memset(&dir_entry[j], 0, sizeof(fat_dir_entry_t));
                    memcpy(dir_entry[j].name, short_name, 11);
                    dir_entry[j].attributes = attributes;

                    /* Ghi sector */
                    if (fat_write_sector(sector + i, dir_buffer) != FAT_SUCCESS) {
                        return FAT_ERROR;
                    }

                    /* Trả về entry */
                    memcpy(entry, &dir_entry[j], sizeof(fat_dir_entry_t));
                    return FAT_SUCCESS;
                }
            }
        }

        /* Lấy cluster tiếp theo */
        if (fat_get_next_cluster(cluster, &cluster) != FAT_SUCCESS) {
            return FAT_ERROR;
        }
    }

    return FAT_ROOT_FULL;
}

/**
 * @brief Xóa entry trong thư mục
 * 
 * @param dir_cluster Cluster của thư mục
 * @param name Tên entry cần xóa
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t fat_delete_directory_entry(uint32_t dir_cluster, const char *name)
{
    uint32_t sector;
    uint32_t cluster = dir_cluster;
    char short_name[12];

    /* Chuyển đổi tên sang định dạng 8.3 */
    if (fat_convert_to_short_name(name, short_name) != FAT_SUCCESS) {
        return FAT_INVALID_NAME;
    }

    /* Duyệt qua các cluster của thư mục */
    while (cluster != FAT_INVALID_CLUSTER) {
        /* Tính sector đầu tiên của cluster */
        sector = fat_ctx.config.first_data_sector + 
                 ((cluster - 2) * fat_ctx.config.sectors_per_cluster);

        /* Duyệt qua các sector trong cluster */
        for (uint32_t i = 0; i < fat_ctx.config.sectors_per_cluster; i++) {
            /* Đọc sector */
            if (fat_read_sector(sector + i, dir_buffer) != FAT_SUCCESS) {
                return FAT_ERROR;
            }

            /* Duyệt qua các entry trong sector */
            fat_dir_entry_t *dir_entry = (fat_dir_entry_t *)dir_buffer;
            for (uint32_t j = 0; j < FAT_SECTOR_SIZE / sizeof(fat_dir_entry_t); j++) {
                /* Kiểm tra entry có hợp lệ không */
                if (dir_entry[j].name[0] == FAT_DIR_EMPTY) {
                    return FAT_NOT_FOUND;
                }
                if (dir_entry[j].name[0] == FAT_DIR_DELETED) {
                    continue;
                }

                /* So sánh tên */
                if (memcmp(dir_entry[j].name, short_name, 11) == 0) {
                    /* Đánh dấu entry đã xóa */
                    dir_entry[j].name[0] = FAT_DIR_DELETED;

                    /* Ghi sector */
                    if (fat_write_sector(sector + i, dir_buffer) != FAT_SUCCESS) {
                        return FAT_ERROR;
                    }

                    return FAT_SUCCESS;
                }
            }
        }

        /* Lấy cluster tiếp theo */
        if (fat_get_next_cluster(cluster, &cluster) != FAT_SUCCESS) {
            return FAT_ERROR;
        }
    }

    return FAT_NOT_FOUND;
}

/*********************************************************************
 * UUID: 5b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 