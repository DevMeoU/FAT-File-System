#ifndef FAT_DRIVER_PRIVATE_H
#define FAT_DRIVER_PRIVATE_H

#include "fat_driver_types.h"
#include "../hal/hal.h"

// Các hằng số và macro cho FAT Driver
#define FAT12_EOC 0xFFF      // End of cluster chain cho FAT12
#define FAT16_EOC 0xFFFF     // End of cluster chain cho FAT16
#define FAT32_EOC 0x0FFFFFFF // End of cluster chain cho FAT32

#define FAT_ATTR_READ_ONLY  0x01
#define FAT_ATTR_HIDDEN     0x02
#define FAT_ATTR_SYSTEM     0x04
#define FAT_ATTR_VOLUME_ID  0x08
#define FAT_ATTR_DIRECTORY  0x10
#define FAT_ATTR_ARCHIVE    0x20
#define FAT_ATTR_LFN        0x0F  // Thuộc tính cho Long File Name

// Cấu trúc entry trong thư mục FAT
typedef struct {
    uint8_t name[8];           // Tên file (8 ký tự)
    uint8_t ext[3];            // Phần mở rộng (3 ký tự)
    uint8_t attributes;        // Thuộc tính file
    uint8_t reserved;          // Dành riêng cho Windows NT
    uint8_t create_time_tenth; // Phần thập phân của thời gian tạo (0-199)
    uint16_t create_time;      // Thời gian tạo
    uint16_t create_date;      // Ngày tạo
    uint16_t last_access_date; // Ngày truy cập cuối
    uint16_t first_cluster_high; // Cluster đầu tiên (high word, chỉ dùng cho FAT32)
    uint16_t write_time;       // Thời gian sửa đổi
    uint16_t write_date;       // Ngày sửa đổi
    uint16_t first_cluster_low; // Cluster đầu tiên (low word)
    uint32_t file_size;        // Kích thước file (byte)
} FATDirEntry;

// Các hàm nội bộ cho FAT Driver
// void fat_driver_parse_boot_sector(FATDriver* driver, const uint8_t* buffer);
// int fat_driver_load_fat_table(FATDriver* driver);
// int fat_driver_load_root_directory(FATDriver* driver);
// int fat_driver_build_directory_tree(FATDriver* driver);
int fat_driver_build_directory_tree_recursive(FATDriver* driver, FileNode* directory);
uint32_t fat_driver_get_next_cluster(FATDriver* driver, uint32_t current_cluster);
uint32_t fat_driver_get_fat_entry(FATDriver* driver, uint32_t cluster);
void fat_driver_fill_file_node(FATDriver* driver, FileNode* node, const FATDirEntry* entry);

#endif // FAT_DRIVER_PRIVATE_H
