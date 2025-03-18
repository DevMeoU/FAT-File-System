#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include "../common/common_types.h"
#include "../fat_driver/fat_driver.h"
#include "../fat_driver/fat_driver_types.h"

typedef struct {
    const char* img_path;
    FileSystemMode mode;
    FATDriver* fat_driver;
    FileNode* current_directory;
    char current_path[256];
    bool is_root_mode;
} Middleware;

/**
 * Khởi tạo Middleware
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_init(Middleware* middleware);

/**
 * Hủy Middleware
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_denit(Middleware* middleware);

/**
 * Xử lý lệnh ls (liệt kê nội dung thư mục)
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_ls(Middleware* middleware);

/**
 * Xử lý lệnh cd (chuyển thư mục)
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @param path Đường dẫn cần chuyển đến
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_cd(Middleware* middleware, const char* path);

/**
 * Xử lý lệnh cat (đọc nội dung file)
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @param path Đường dẫn file cần đọc
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_cat(Middleware* middleware, const char* path);

/**
 * Xử lý lệnh evidence (hiển thị thông tin hệ thống tệp)
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_evidence(Middleware* middleware);

/**
 * Chuyển sang chế độ root
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_switch_to_root_mode(Middleware* middleware);

/**
 * Chuyển sang chế độ user
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int middleware_switch_to_user_mode(Middleware* middleware);

/**
 * Lấy đường dẫn hiện tại
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return Chuỗi đường dẫn hiện tại
 */
const char* middleware_get_current_path(Middleware* middleware);

/**
 * Kiểm tra có đang ở chế độ root không
 * @param middleware Con trỏ đến cấu trúc Middleware
 * @return true nếu đang ở chế độ root, false nếu không
 */
bool middleware_is_root_mode(Middleware* middleware);

#endif // MIDDLEWARE_H
