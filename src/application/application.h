/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Application cung cấp giao diện người dùng để tương tác với
 *   hệ thống FAT, cho phép người dùng thực hiện các thao tác như xem
 *   danh sách file, đọc/ghi file, tạo/xóa thư mục.
 *********************************************************************/
#ifndef APPLICATION_H
#define APPLICATION_H

#include "../middleware/middleware.h"
#include "../common/common_types.h"

/* Application Error Codes */
#define APP_SUCCESS 0
#define APP_ERROR -1
#define APP_INVALID -2
#define APP_NOT_FOUND -3

/* Application Types */
typedef struct {
    Middleware* mw;
    int running;
} Application;

/* Public Function Prototypes */
int application_init(Application* app, const char* img_path);
void application_run_shell(Application* app);
int application_handle_command(Application* app, const char* command);
void application_cleanup(Application* app);

#endif

/*********************************************************************
 * UUID: 9b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/

