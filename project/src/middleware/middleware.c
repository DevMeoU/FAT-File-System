/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#include "middleware.h"
#include "fat_driver.h"
#include <stdio.h>
#include <string.h>

void middleware_list_directory(const char *path) {
    printf("Danh sách file/thư mục tại %s:\n", path);
    // Gọi FAT Driver để liệt kê thư mục tại path
    fat_driver_list_directory(path);  // Truyền string path trực tiếp
    // In kết quả (giả sử FAT Driver xử lý việc hiển thị)
}

void middleware_change_directory(const char *path, char *current_path) {
    // Kiểm tra và cập nhật đường dẫn
    if (fat_driver_directory_exists(path)) {
        strcpy(current_path, path);
        printf("Đã chuyển đến thư mục: %s\n", path);
    } else {
        printf("Thư mục không tồn tại: %s\n", path);
    }
}

void middleware_read_file(const char *filename) {
    // Gọi FAT Driver để đọc file
    printf("Nội dung của file %s:\n", filename);
    fat_driver_read_file(filename);  // Truyền string filename trực tiếp
    // In kết quả (giả sử FAT Driver xử lý việc hiển thị)
}