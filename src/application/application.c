/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Application cung cấp giao diện người dùng để tương tác với
 *   hệ thống FAT, cho phép người dùng thực hiện các thao tác như xem
 *   danh sách file, đọc/ghi file, tạo/xóa thư mục.
 *********************************************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "application.h"
#include "../middleware/middleware.h"
#include "../utilities/log/print_color.h"
#include "../common/common_types.h"
#include "../fat_driver/fat_driver.h"
#include "../ip_driver/ip_driver.h"
#include "../hal/hal.h"

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/* Forward declarations for command handlers */
static int32_t app_cmd_help_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_list_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_cd_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_mkdir_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_rmdir_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_cat_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_write_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_rm_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_cp_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_mv_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_mount_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_exit_handler(Application* app, int argc, char *argv[]);
static int32_t app_cmd_clear_handler(Application* app, int argc, char *argv[]);

/*********************************************************************
 * Private Variables
 *********************************************************************/

/* Current directory */
static char app_current_dir[APP_PATH_BUF_SIZE] = "/";

/* Command table */
static const struct {
    const char *name;
    const char *desc;
    const char *usage;
    int32_t (*handler)(Application* app, int argc, char *argv[]);
} app_cmd_table[] = {
    {"help",  "Hiển thị trợ giúp", "help [command]", app_cmd_help_handler},
    {"ls",    "Liệt kê thư mục", "ls [path]", app_cmd_list_handler},
    {"cd",    "Thay đổi thư mục", "cd <path>", app_cmd_cd_handler},
    {"mkdir", "Tạo thư mục mới", "mkdir <path>", app_cmd_mkdir_handler},
    {"rmdir", "Xóa thư mục", "rmdir <path>", app_cmd_rmdir_handler},
    {"cat",   "Xem nội dung file", "cat <file>", app_cmd_cat_handler},
    {"write", "Ghi nội dung vào file", "write <file> <content>", app_cmd_write_handler},
    {"rm",    "Xóa file", "rm <file>", app_cmd_rm_handler},
    {"cp",    "Sao chép file", "cp <source> <destination>", app_cmd_cp_handler},
    {"mv",    "Di chuyển/đổi tên file", "mv <source> <destination>", app_cmd_mv_handler},
    {"mount", "Mount file system", "mount <file>", app_cmd_mount_handler},
    {"exit",  "Thoát chương trình", "exit", app_cmd_exit_handler},
    {"cls", "Xóa màn hình", "clear", app_cmd_clear_handler},
    {NULL, NULL, NULL, NULL}
};

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int application_init(Application* app, const char* img_path) {
    if (!app || !img_path) {
        return STATUS_INVALID_PARAMETER;
    }

    /* Initialize IP driver */
    ip_config_t ip_config = {
        .img_path = img_path,
        .sector_size = 512,
        .total_sectors = 0,
        .current_sector = 0
    };

    if (ip_driver_init(&ip_config) != IP_ERROR_SUCCESS) {
        printf("Failed to initialize IP driver\n");
        return STATUS_ERROR;
    }

    /* Initialize HAL */
    hal_config_t hal_config = {
        .driver = NULL,
        .sector_size = 512,
        .cache_size = 16
    };

    if (hal_init(&hal_config) != HAL_ERROR_SUCCESS) {
        printf("Failed to initialize HAL\n");
        ip_driver_deinit();
        return STATUS_ERROR;
    }

    /* Initialize FAT driver */
    if (fat_init(img_path) != FAT_ERROR_SUCCESS) {
        printf("Failed to initialize FAT driver\n");
        hal_deinit();
        ip_driver_deinit();
        return STATUS_ERROR;
    }

    // Khởi tạo Middleware
    Middleware* mw = malloc(sizeof(Middleware));
    if (!mw) {
        fat_deinit();
        hal_deinit();
        ip_driver_deinit();
        return APP_ERROR;
    }

    if (middleware_init(mw) != MID_SUCCESS) {
        free(mw);
        fat_deinit();
        hal_deinit();
        ip_driver_deinit();
        return APP_ERROR;
    }

    app->mw = mw;
    app->running = 1;
    strncpy(app_current_dir, "/", sizeof(app_current_dir) - 1);
    app_current_dir[sizeof(app_current_dir) - 1] = '\0';
    return APP_SUCCESS;
}

void application_run_shell(Application* app) {
    char command[1024];
    
    while (app->running) {
        printf("%s> ", app_current_dir);
        if (fgets(command, sizeof(command), stdin)) {
            // Xóa ký tự newline
            command[strcspn(command, "\n")] = 0;
            
            if (application_handle_command(app, command) != APP_SUCCESS) {
                printf("Lệnh không hợp lệ\n");
            }
        }
    }
}

int application_handle_command(Application* app, const char* command) {
    if (!app || !command) return APP_ERROR;

    char cmd[32];
    char arg[992];
    
    // Tách lệnh và tham số
    if (sscanf(command, "%31s %991s", cmd, arg) < 1) {
        return APP_ERROR;
    }

    // Tìm và thực thi lệnh
    for (size_t i = 0; app_cmd_table[i].name != NULL; i++) {
        if (strcmp(cmd, app_cmd_table[i].name) == 0) {
            char* argv[32];
            int argc = 1;
            argv[0] = cmd;

            // Tách tham số
            char* token = strtok(arg, " ");
            while (token && argc < 32) {
                argv[argc++] = token;
                token = strtok(NULL, " ");
            }

            return app_cmd_table[i].handler(app, argc, argv);
        }
    }

    return APP_ERROR;
}

void application_cleanup(Application* app) {
    if (app) {
        if (app->mw) {
            middleware_cleanup(app->mw);
            free(app->mw);
        }
        fat_deinit();
        hal_deinit();
        ip_driver_deinit();
    }
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

int32_t app_cmd_help_handler(Application* app, int argc, char *argv[]) {
    (void)app; // Unused parameter
    if (argc < 2) {
        /* Hiển thị tất cả lệnh */
        printf("Available commands:\n");
        for (size_t i = 0; app_cmd_table[i].name != NULL; i++) {
            printf("  %-10s - %s\n", app_cmd_table[i].name, app_cmd_table[i].desc);
        }
        return APP_SUCCESS;
    } else {
        /* Tìm lệnh trong bảng */
        for (size_t i = 0; app_cmd_table[i].name != NULL; i++) {
            if (strcmp(argv[1], app_cmd_table[i].name) == 0) {
                printf("Usage: %s\n", app_cmd_table[i].usage);
                printf("Description: %s\n", app_cmd_table[i].desc);
                return APP_SUCCESS;
            }
        }
        return APP_NOT_FOUND;
    }
}

int32_t app_cmd_list_handler(Application* app, int argc, char *argv[]) {
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter
    return middleware_list_directory(app->mw);
}

int32_t app_cmd_cd_handler(Application* app, int argc, char *argv[]) {
    if (argc < 2) {
        return APP_INVALID;
    }
    int32_t result = middleware_change_directory(app->mw, argv[1]);
    if (result == APP_SUCCESS) {
        strncpy(app_current_dir, argv[1], sizeof(app_current_dir) - 1);
        app_current_dir[sizeof(app_current_dir) - 1] = '\0';
    }
    return result;
}

int32_t app_cmd_mkdir_handler(Application* app, int argc, char *argv[]) {
    if (argc < 2) {
        return APP_INVALID;
    }
    return middleware_create_directory(app->mw, argv[1]);
}

int32_t app_cmd_rmdir_handler(Application* app, int argc, char *argv[]) {
    if (argc < 2) {
        return APP_INVALID;
    }
    return middleware_remove_directory(app->mw, argv[1]);
}

int32_t app_cmd_cat_handler(Application* app, int argc, char *argv[]) {
    if (argc < 2) {
        return APP_INVALID;
    }
    return middleware_read_file(app->mw, argv[1]);
}

int32_t app_cmd_write_handler(Application* app, int argc, char *argv[]) {
    if (argc < 3) {
        return APP_INVALID;
    }
    return middleware_write_file(app->mw, argv[1], argv[2]);
}

int32_t app_cmd_rm_handler(Application* app, int argc, char *argv[]) {
    if (argc < 2) {
        return APP_INVALID;
    }
    return middleware_remove_file(app->mw, argv[1]);
}

int32_t app_cmd_cp_handler(Application* app, int argc, char *argv[]) {
    if (argc < 3) {
        return APP_INVALID;
    }
    return middleware_copy_file(app->mw, argv[1], argv[2]);
}

int32_t app_cmd_mv_handler(Application* app, int argc, char *argv[]) {
    if (argc < 3) {
        return APP_INVALID;
    }
    return middleware_move_file(app->mw, argv[1], argv[2]);
}

int32_t app_cmd_mount_handler(Application* app, int argc, char *argv[]) {
    if (argc < 2) {
        return APP_INVALID;
    }
    return middleware_mount(app->mw, argv[1]);
}

int32_t app_cmd_exit_handler(Application* app, int argc, char *argv[]) {
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter
    app->running = 0;
    return APP_SUCCESS;
}

int32_t app_cmd_clear_handler(Application* app, int argc, char *argv[]) {
    (void)app; // Unused parameter
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter
    system("clear");
    return APP_SUCCESS;
}

/*********************************************************************
 * Main Function
 *********************************************************************/

int main(int argc, char *argv[]) {
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter
    Application app;
    if (application_init(&app, NULL) != APP_SUCCESS) {
        return 1;
    }

    application_run_shell(&app);
    application_cleanup(&app);

    return 0;
}

/*********************************************************************
 * UUID: 4f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/