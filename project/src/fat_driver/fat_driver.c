/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "fat_driver.h"
#include "hal.h"

// Định nghĩa cấu trúc cho thông tin boot sector
typedef struct {
    unsigned short bytes_per_sector;    // Số byte trên mỗi sector
    unsigned char sectors_per_cluster;  // Số sector trên mỗi cluster
    unsigned short reserved_sectors;    // Số sector dành sẵn
    unsigned char number_of_fats;       // Số bảng FAT
    unsigned short root_dir_entries;    // Số entry trong thư mục gốc
    unsigned short total_sectors;       // Tổng số sector
} BootSectorInfo;

// Biến toàn cục lưu thông tin boot sector
BootSectorInfo boot_info;

// Hàm khởi tạo FAT Driver
int fat_driver_init(const char *img_path) {
    // Khởi tạo HAL với đường dẫn đến file floppy.img
    if (hal_init(img_path) != 0) {
        printf("Lỗi: Không thể khởi tạo HAL.\n");
        return -1;
    }
    
    // Đọc và phân tích boot sector
    if (fat_driver_read_boot_sector() != 0) {
        printf("Lỗi: Không thể đọc boot sector.\n");
        return -1;
    }
    
    return 0;  // Thành công
}

// Hàm đọc và phân tích boot sector
int fat_driver_read_boot_sector() {
    unsigned char buffer[512];
    
    // Đọc sector 0 (boot sector) từ file
    if (hal_read_sector(0, buffer) != 0) {
        return -1;
    }
    
    // Phân tích các byte trong boot sector
    boot_info.bytes_per_sector = *(unsigned short*)(buffer + 11);  // Offset 11
    boot_info.sectors_per_cluster = buffer[13];                    // Offset 13
    boot_info.reserved_sectors = *(unsigned short*)(buffer + 14);  // Offset 14
    boot_info.number_of_fats = buffer[16];                         // Offset 16
    boot_info.root_dir_entries = *(unsigned short*)(buffer + 17);  // Offset 17
    boot_info.total_sectors = *(unsigned short*)(buffer + 19);     // Offset 19
    
    // Kiểm tra tính hợp lệ của thông tin
    if (boot_info.bytes_per_sector != 512) {
        printf("Cảnh báo: Kích thước sector không phải 512 byte.\n");
    }
    
    return 0;  // Thành công
}

// Các hàm hiện có từ mã nguồn gốc
int fat_driver_list_directory(const char *path) {
    // Logic phân tích path để tìm thư mục
    // Đọc sector chứa thư mục qua HAL, phân tích các entry
    // Ví dụ: In trực tiếp danh sách file/thư mục
    unsigned char buffer[512];
    hal_read_sector(1, buffer);  // Giả sử sector 1 chứa thư mục gốc
    // Xử lý và in các entry (tên file, kích thước, v.v.)
    printf("file1.txt  512 bytes\n");  // Ví dụ minh họa
    printf("dir1      <DIR>\n");
    return 0;  // Thành công
}

int fat_driver_read_file(const char *filename) {
    // Logic tìm file theo filename trong thư mục hiện tại
    // Đọc chuỗi cluster qua HAL và in nội dung
    unsigned char buffer[512];
    hal_read_sector(2, buffer);  // Giả sử sector 2 chứa dữ liệu file
    printf("%s\n", buffer);  // In nội dung file
    return 0;  // Thành công
}

int fat_driver_directory_exists(const char *path) {
    // Logic kiểm tra path có phải thư mục hợp lệ
    // Đọc sector chứa thư mục, kiểm tra entry
    return 1;  // Giả sử tồn tại (cần triển khai thực tế)
}

int fat_driver_get_next_cluster(uint16_t current_cluster) {
    // Tính offset trong bảng FAT (1.5 byte mỗi entry)
    uint32_t fat_offset = (current_cluster * 3) / 2;
    uint16_t sector_size = boot_info.bytes_per_sector;
    uint16_t fat_start_sector = boot_info.reserved_sectors;
    uint16_t sector_containing_entry = fat_start_sector + (fat_offset / sector_size);
    uint16_t offset_in_sector = fat_offset % sector_size;

    unsigned char buffer[512];
    if (hal_read_sector(sector_containing_entry, buffer) != 0) {
        printf("Lỗi: Không thể đọc sector %d của bảng FAT.\n", sector_containing_entry);
        return -1;
    }

    uint16_t entry_value;
    if (current_cluster % 2 == 0) {
        entry_value = *(uint16_t*)(buffer + offset_in_sector) & 0x0FFF;  // Cluster chẵn
    } else {
        entry_value = (*(uint16_t*)(buffer + offset_in_sector) >> 4) & 0x0FFF;  // Cluster lẻ
    }

    if (entry_value >= 0xFF8) {
        return -1;  // EOF
    } else if (entry_value == 0xFF7) {
        printf("Cluster bị hỏng: %d\n", current_cluster);
        return -1;
    } else {
        return entry_value;  // Cluster tiếp theo
    }
}