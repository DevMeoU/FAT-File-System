#ifndef FAT_DRIVER_TYPES_H
#define FAT_DRIVER_TYPES_H

#include <stdint.h>
#include "../common/common_types.h"

// Cấu trúc Boot Sector
typedef struct {
    uint16_t bytes_per_sector;      // Offset 11-12: Số byte trong một sector
    uint8_t sectors_per_cluster;    // Offset 13: Số sector trong một cluster
    uint16_t reserved_sectors;      // Offset 14-15: Số sector dự phòng
    uint8_t number_of_fats;         // Offset 16: Số lượng bảng FAT
    uint16_t root_entry_count;      // Offset 17-18: Số lượng entry trong root directory (FAT12/16)
    uint16_t total_sectors_16;      // Offset 19-20: Tổng số sector (16-bit)
    uint8_t media_type;             // Offset 21: Loại media
    uint16_t fat_size_16;           // Offset 22-23: Số sector cho mỗi bảng FAT (FAT12/16)
    uint16_t sectors_per_track;     // Offset 24-25: Số sector trên mỗi track
    uint16_t number_of_heads;       // Offset 26-27: Số đầu đọc
    uint32_t hidden_sectors;        // Offset 28-31: Số sector ẩn
    uint32_t total_sectors_32;      // Offset 32-35: Tổng số sector (32-bit)
    
    // Các trường bổ sung cho FAT32
    uint32_t fat_size_32;           // Offset 36-39: Số sector cho mỗi bảng FAT (FAT32)
    uint16_t extended_flags;        // Offset 40-41: Cờ mở rộng
    uint16_t fs_version;            // Offset 42-43: Phiên bản hệ thống tệp
    uint32_t root_cluster;          // Offset 44-47: Cluster bắt đầu của root directory
    uint16_t fs_info;               // Offset 48-49: Sector chứa thông tin FS_INFO
    uint16_t backup_boot_sector;    // Offset 50-51: Sector dự phòng cho boot sector
    uint8_t reserved[12];           // Offset 52-63: Dự phòng
    uint8_t drive_number;           // Offset 64: Số ổ đĩa
    uint8_t reserved1;              // Offset 65: Dự phòng
    uint8_t boot_signature;         // Offset 66: Chữ ký boot
    uint32_t volume_id;             // Offset 67-70: ID volume
    char volume_label[11];          // Offset 71-81: Nhãn volume
    char fs_type[8];                // Offset 82-89: Loại hệ thống tệp
} BootSector;

// Cấu trúc Directory Entry
typedef struct {
    char name[8];                   // Tên file (8 ký tự)
    char extension[3];              // Phần mở rộng (3 ký tự)
    uint8_t attributes;             // Thuộc tính file
    uint8_t reserved;               // Dự phòng
    uint8_t creation_time_tenth;    // Phần thập phân của thời gian tạo
    uint16_t creation_time;         // Thời gian tạo
    uint16_t creation_date;         // Ngày tạo
    uint16_t last_access_date;      // Ngày truy cập cuối
    uint16_t first_cluster_high;    // Phần cao của cluster đầu tiên (FAT32)
    uint16_t last_modification_time; // Thời gian sửa đổi cuối
    uint16_t last_modification_date; // Ngày sửa đổi cuối
    uint16_t first_cluster_low;     // Phần thấp của cluster đầu tiên
    uint32_t file_size;             // Kích thước file
} DirectoryEntry;

// Cấu trúc LFN (Long File Name) Entry
typedef struct {
    uint8_t order;                  // Thứ tự của entry
    uint16_t name1[5];              // 5 ký tự đầu tiên (Unicode)
    uint8_t attribute;              // Thuộc tính (luôn là 0x0F cho LFN)
    uint8_t type;                   // Loại (0 cho LFN)
    uint8_t checksum;               // Checksum
    uint16_t name2[6];              // 6 ký tự tiếp theo (Unicode)
    uint16_t first_cluster;         // Luôn là 0 cho LFN
    uint16_t name3[2];              // 2 ký tự cuối (Unicode)
} LFNEntry;

// Cấu trúc File/Directory
typedef struct FileNode {
    char name[FILE_NAME_MAX + 1];   // Tên file/thư mục
    FileType type;                  // Loại (file/thư mục)
    FileAttributes attributes;      // Thuộc tính
    uint32_t size;                  // Kích thước
    uint32_t first_cluster;         // Cluster đầu tiên
    uint32_t first_sector;          // Sector đầu tiên
    DateTime created_time;          // Thời gian tạo
    DateTime modified_time;         // Thời gian sửa đổi
    struct FileNode* parent;        // Thư mục cha
    struct FileNode* children;      // Danh sách con (nếu là thư mục)
    struct FileNode* next;          // Node tiếp theo trong cùng thư mục
} FileNode;

// Cấu trúc FAT Driver
typedef struct {
    HAL* hal;                       // Con trỏ đến HAL
    FileSystemConfig config;        // Cấu hình hệ thống
    BootSector boot_sector;         // Boot sector
    uint32_t* fat_table;            // Bảng FAT
    uint32_t first_fat_sector;      // Sector đầu tiên của bảng FAT
    uint32_t first_data_sector;     // Sector đầu tiên của vùng dữ liệu
    uint32_t root_dir_sectors;      // Số sector của thư mục gốc (FAT12/16)
    uint32_t first_root_dir_sector; // Sector đầu tiên của thư mục gốc (FAT12/16)
    uint32_t data_sectors;          // Số sector dữ liệu
    uint32_t total_clusters;        // Tổng số cluster
    FileNode* root_directory;       // Thư mục gốc
    FileNode* current_directory;    // Thư mục hiện tại
    void* cache;                    // Cache
    uint32_t cache_size;            // Kích thước cache
} FATDriver;

#endif // FAT_DRIVER_TYPES_H
