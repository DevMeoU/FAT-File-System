/*
* Middleware Module
* Author: Ducson9112k
*
* Description:
*   Cung cấp các hàm trung gian để khởi tạo hệ thống, liệt kê thư mục, thay đổi thư mục và đọc file
*   dựa trên FAT Driver.
*/

#include "middleware.h"
#include "fat_driver.h"
#include "linkedlist.h"
#include <stdio.h>
#include <string.h>

/* Global variable: danh sách đường dẫn (cây thư mục) */
static linkedlist_t *path_list = NULL;

/*
* middleware_init:
*   Khởi tạo middleware và FAT driver.
*   Cấp phát danh sách liên kết cho cây thư mục và khởi tạo FAT driver.
*/
int middleware_init(const char *img_path) {
    path_list = llist_init();
    if (path_list == NULL) {
        printf("Failed to initialize linked list.\n");
        return -1;
    }
    if (fat_driver_init(img_path, path_list) != 0) {
        printf("Failed to initialize FAT driver.\n");
        return -1;
    }
    return 0;
}

/*
* middleware_list_directory:
*   Liệt kê nội dung thư mục tại đường dẫn được chỉ định.
*   Hàm gọi FAT Driver để hiển thị danh sách.
*/
void middleware_list_directory(const char *path) {
    if (fat_driver_list_directory(path, path_list) != 0) {
        printf("Failed to list directory.\n");
    }
}

/*
* middleware_change_directory:
*   Thay đổi thư mục hiện tại.
*   Nếu thư mục tồn tại, cập nhật current_path; nếu không, thông báo lỗi.
*/
void middleware_change_directory(const char *path, char *current_path) {
    if (fat_driver_directory_exists(path)) {
        /* Ví dụ: nối đường dẫn hợp lệ, cập nhật current_path */
        strcpy(current_path, path);
        if (current_path[strlen(current_path) - 1] != '/')
            strcat(current_path, "/");
        printf("Changed to directory: %s\n", current_path);
    } else {
        printf("Directory does not exist: %s\n", path);
    }
}

/*
* middleware_read_file:
*   Đọc nội dung file theo đường dẫn được chỉ định.
*   Gọi FAT Driver để thực hiện việc đọc và hiển thị nội dung file.
*/
void middleware_read_file(const char *filename) {
    if (fat_driver_read_file(filename, path_list) != 0) {
        printf("Failed to read file.\n");
    }
}
