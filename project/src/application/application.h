#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utilities/log/print_color.h"
#include "../common/common_types.h"
#include "../middleware/middleware.h"

typedef struct {
    Middleware* middleware;
    char env_path[500];
    bool running;
} Application;

/**
 * Khởi tạo Application
 * @param app Con trỏ đến cấu trúc Application
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int application_init(Application* app, Middleware* middleware);

/**
 * Chạy ứng dụng
 * @param app Con trỏ đến cấu trúc Application
 * @param img_path Đường dẫn đến file ảnh
 * @param mode Chế độ (read-only hoặc read-write)
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int application_run(Application* app);

/**
 * Xử lý lệnh từ người dùng
 * @param app Con trỏ đến cấu trúc Application
 * @param command Lệnh cần xử lý
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int application_process_command(Application* app, const char* command);

/**
 * Hiển thị trợ giúp
 * @param app Con trỏ đến cấu trúc Application
 */
void application_show_help(Application* app);

/**
 * Dừng ứng dụng
 * @param app Con trỏ đến cấu trúc Application
 */
void application_stop(Application* app);

#endif // APPLICATION_H
