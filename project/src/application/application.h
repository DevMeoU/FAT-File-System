#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
 * Hiển thị prompt
 * @param app Con trỏ đến cấu trúc Application
 */
void display_prompt(Application* app);

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
char* trim(char* str);

/**
 * Hàm thay thế strtok_r()
 * @param str Chuỗi cần tách
 * @param delim Ký tự phân cách
 * @param saveptr Con trỏ lưu vị trí tách
 * @return Chuỗi tách được
 */
char *custom_strtok_r(char *str, const char *delim, char **saveptr);

/**
 * Xử lý lệnh từ người dùng
 * @param app Con trỏ đến cấu trúc Application
 * @param command Lệnh cần xử lý
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int process_command_with_and(Application* app, const char* command);

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
