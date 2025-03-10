/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Middleware cung cấp lớp trung gian giữa tầng ứng dụng và
 *   tầng driver, xử lý các yêu cầu từ ứng dụng và chuyển đổi thành
 *   các lệnh phù hợp cho driver.
 *********************************************************************/
#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#include <stdint.h>
#include "../common/common_types.h"
#include "../fat_driver/fat_driver.h"

/* Middleware Error Codes */
#define MID_SUCCESS 0
#define MID_ERROR -1
#define MID_INVALID -2
#define MID_NOT_FOUND -3

/* Middleware Types */
typedef struct {
    FATDriver* fat_driver;
    char current_path[1024];
} Middleware;

/* Public Function Prototypes */
int32_t middleware_init(Middleware* mw);
int32_t middleware_cleanup(Middleware* mw);
int32_t middleware_list_directory(Middleware* mw);
int32_t middleware_change_directory(Middleware* mw, const char* path);
int32_t middleware_create_directory(Middleware* mw, const char* path);
int32_t middleware_remove_directory(Middleware* mw, const char* path);
int32_t middleware_read_file(Middleware* mw, const char* path);
int32_t middleware_write_file(Middleware* mw, const char* path, const char* content);
int32_t middleware_remove_file(Middleware* mw, const char* path);
int32_t middleware_copy_file(Middleware* mw, const char* src, const char* dst);
int32_t middleware_move_file(Middleware* mw, const char* src, const char* dst);
int32_t middleware_mount(Middleware* mw, const char* path);

#endif /* MIDDLEWARE_H */

/*********************************************************************
 * UUID: 3b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
