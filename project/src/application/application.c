/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Application cung cấp giao diện người dùng để tương tác với
 *   hệ thống FAT, cho phép người dùng thực hiện các thao tác như xem
 *   danh sách file, đọc/ghi file, tạo/xóa thư mục.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "application.h"
#include "../middleware/middleware.h"
#include "../utilities/log/print_color.h"
#include "../utilities/status/common_type.h"

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/* Forward declarations for command handlers */
static int32_t app_cmd_help_handler(int argc, char *argv[]);
static int32_t app_cmd_list_handler(int argc, char *argv[]);
static int32_t app_cmd_cd_handler(int argc, char *argv[]);
static int32_t app_cmd_mkdir_handler(int argc, char *argv[]);
static int32_t app_cmd_rmdir_handler(int argc, char *argv[]);
static int32_t app_cmd_cat_handler(int argc, char *argv[]);
static int32_t app_cmd_write_handler(int argc, char *argv[]);
static int32_t app_cmd_rm_handler(int argc, char *argv[]);
static int32_t app_cmd_cp_handler(int argc, char *argv[]);
static int32_t app_cmd_mv_handler(int argc, char *argv[]);
static int32_t app_cmd_exit_handler(int argc, char *argv[]);

/*********************************************************************
 * Private Variables
 *********************************************************************/

/* Current directory */
static char app_current_dir[APP_PATH_BUF_SIZE] = "/";

/* Command buffer */
static char app_cmd_buffer[APP_CMD_BUF_SIZE];

/* Data buffer */
static uint8_t app_data_buffer[APP_DATA_BUF_SIZE];

/* Command table */
static const app_cmd_t app_cmd_table[] = {
    {
        .name = APP_CMD_HELP,
        .desc = APP_CMD_HELP_DESC,
        .usage = APP_CMD_HELP_USAGE,
        .handler = app_cmd_help_handler
    },
    {
        .name = APP_CMD_LIST,
        .desc = APP_CMD_LIST_DESC,
        .usage = APP_CMD_LIST_USAGE,
        .handler = app_cmd_list_handler
    },
    {
        .name = APP_CMD_CHANGE_DIR,
        .desc = APP_CMD_CHANGE_DIR_DESC,
        .usage = APP_CMD_CHANGE_DIR_USAGE,
        .handler = app_cmd_cd_handler
    },
    {
        .name = APP_CMD_MAKE_DIR,
        .desc = APP_CMD_MAKE_DIR_DESC,
        .usage = APP_CMD_MAKE_DIR_USAGE,
        .handler = app_cmd_mkdir_handler
    },
    {
        .name = APP_CMD_REMOVE_DIR,
        .desc = APP_CMD_REMOVE_DIR_DESC,
        .usage = APP_CMD_REMOVE_DIR_USAGE,
        .handler = app_cmd_rmdir_handler
    },
    {
        .name = APP_CMD_READ,
        .desc = APP_CMD_READ_DESC,
        .usage = APP_CMD_READ_USAGE,
        .handler = app_cmd_cat_handler
    },
    {
        .name = APP_CMD_WRITE,
        .desc = APP_CMD_WRITE_DESC,
        .usage = APP_CMD_WRITE_USAGE,
        .handler = app_cmd_write_handler
    },
    {
        .name = APP_CMD_DELETE,
        .desc = APP_CMD_DELETE_DESC,
        .usage = APP_CMD_DELETE_USAGE,
        .handler = app_cmd_rm_handler
    },
    {
        .name = APP_CMD_COPY,
        .desc = APP_CMD_COPY_DESC,
        .usage = APP_CMD_COPY_USAGE,
        .handler = app_cmd_cp_handler
    },
    {
        .name = APP_CMD_MOVE,
        .desc = APP_CMD_MOVE_DESC,
        .usage = APP_CMD_MOVE_USAGE,
        .handler = app_cmd_mv_handler
    },
    {
        .name = APP_CMD_EXIT,
        .desc = APP_CMD_EXIT_DESC,
        .usage = APP_CMD_EXIT_USAGE,
        .handler = app_cmd_exit_handler
    }
};

/* Running flag */
static bool app_is_running = false;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t app_init(void)
{
    /* Khởi tạo middleware */
    if (mid_init() != MID_SUCCESS) {
        return APP_ERROR;
    }

    /* Khởi tạo biến */
    memset(app_cmd_buffer, 0, sizeof(app_cmd_buffer));
    memset(app_data_buffer, 0, sizeof(app_data_buffer));
    app_is_running = true;

    return APP_SUCCESS;
}

int32_t app_run(void)
{
    /* In thông tin chào mừng */
    log_info("FAT File System Manager");
    log_info("Type 'help' for more information");

    /* Vòng lặp chính */
    while (app_is_running) {
        /* In prompt */
        printf("%s> ", app_current_dir);

        /* Đọc lệnh */
        if (fgets(app_cmd_buffer, sizeof(app_cmd_buffer), stdin) == NULL) {
            break;
        }

        /* Xóa ký tự xuống dòng */
        app_cmd_buffer[strcspn(app_cmd_buffer, "\n")] = 0;

        /* Xử lý lệnh */
        if (app_process_command(app_cmd_buffer) != APP_SUCCESS) {
            log_error("Command failed");
        }
    }

    return APP_SUCCESS;
}

int32_t app_process_command(const char *cmd_line)
{
    if (cmd_line == NULL) {
        return APP_INVALID;
    }

    /* Tách lệnh và tham số */
    char *argv[16];
    int argc = 0;
    char *token = strtok((char *)cmd_line, " ");
    while (token != NULL && argc < 16) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    if (argc == 0) {
        return APP_SUCCESS;
    }

    /* Tìm lệnh trong bảng */
    for (size_t i = 0; i < sizeof(app_cmd_table)/sizeof(app_cmd_t); i++) {
        if (strcmp(argv[0], app_cmd_table[i].name) == 0) {
            return app_cmd_table[i].handler(argc, argv);
        }
    }

    log_error("Unknown command: %s", argv[0]);
    return APP_INVALID_CMD;
}

int32_t app_show_help(const char *cmd)
{
    if (cmd == NULL) {
        /* Hiển thị tất cả lệnh */
        log_info("Available commands:");
        for (size_t i = 0; i < sizeof(app_cmd_table)/sizeof(app_cmd_t); i++) {
            printf("  %-10s - %s\n", app_cmd_table[i].name, app_cmd_table[i].desc);
        }
    } else {
        /* Tìm lệnh trong bảng */
        for (size_t i = 0; i < sizeof(app_cmd_table)/sizeof(app_cmd_t); i++) {
            if (strcmp(cmd, app_cmd_table[i].name) == 0) {
                printf("Usage: %s\n", app_cmd_table[i].usage);
                printf("Description: %s\n", app_cmd_table[i].desc);
                return APP_SUCCESS;
            }
        }
        return APP_NOT_FOUND;
    }

    return APP_SUCCESS;
}

int32_t app_list_directory(const char *path)
{
    const char *target_path = (path != NULL) ? path : app_current_dir;

    /* Đọc thông tin thư mục */
    fat_file_t dir;
    if (fat_open(target_path, FAT_MODE_READ, &dir) != FAT_SUCCESS) {
        return APP_ERROR;
    }

    /* Đọc các entry */
    fat_dir_entry_t entry;
    uint32_t bytes_read;
    while (fat_read(&dir, &entry, sizeof(entry), &bytes_read) == FAT_SUCCESS) {
        if (bytes_read == 0) {
            break;
        }

        /* Hiển thị thông tin */
        if (entry.attributes & FAT_ATTR_DIRECTORY) {
            printf("[DIR]  %s\n", entry.name);
        } else {
            printf("[FILE] %s (%u bytes)\n", entry.name, entry.file_size);
        }
    }

    fat_close(&dir);
    return APP_SUCCESS;
}

int32_t app_change_directory(const char *path)
{
    if (path == NULL) {
        return APP_INVALID;
    }

    /* Kiểm tra thư mục tồn tại */
    fat_file_t dir;
    if (fat_open(path, FAT_MODE_READ, &dir) != FAT_SUCCESS) {
        return APP_NOT_FOUND;
    }

    /* Kiểm tra là thư mục */
    fat_file_info_t info;
    if (fat_stat(path, &info) != FAT_SUCCESS || !(info.attributes & FAT_ATTR_DIRECTORY)) {
        fat_close(&dir);
        return APP_ERROR;
    }

    fat_close(&dir);

    /* Cập nhật đường dẫn hiện tại */
    strncpy(app_current_dir, path, sizeof(app_current_dir) - 1);
    app_current_dir[sizeof(app_current_dir) - 1] = '\0';

    return APP_SUCCESS;
}

int32_t app_read_file(const char *path)
{
    if (path == NULL) {
        return APP_INVALID;
    }

    /* Mở file */
    fat_file_t file;
    if (fat_open(path, FAT_MODE_READ, &file) != FAT_SUCCESS) {
        return APP_NOT_FOUND;
    }

    /* Đọc và hiển thị nội dung */
    uint32_t bytes_read;
    uint8_t buffer[APP_DATA_BUF_SIZE];
    int32_t status = APP_SUCCESS;

    while ((status = fat_read(&file, buffer, sizeof(buffer), &bytes_read)) == FAT_SUCCESS) {
        if (bytes_read == 0) {
            break;
        }
        fwrite(buffer, 1, bytes_read, stdout);
    }

    fat_close(&file);
    printf("\n");

    return status;
}

int32_t app_write_file(const char *path, const void *data, uint32_t size)
{
    if (path == NULL || data == NULL) {
        return APP_INVALID;
    }

    /* Mở file */
    fat_file_t file;
    if (fat_open(path, FAT_MODE_WRITE | FAT_MODE_CREATE | FAT_MODE_TRUNCATE, &file) != FAT_SUCCESS) {
        return APP_ERROR;
    }

    /* Ghi dữ liệu */
    uint32_t bytes_written;
    int32_t status = fat_write(&file, data, size, &bytes_written);

    fat_close(&file);

    return (status == FAT_SUCCESS) ? APP_SUCCESS : APP_ERROR;
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t app_cmd_help_handler(int argc, char *argv[])
{
    if (argc > 2) {
        log_error("Too many arguments");
        return APP_INVALID_ARG;
    }

    return app_show_help((argc == 2) ? argv[1] : NULL);
}

static int32_t app_cmd_list_handler(int argc, char *argv[])
{
    if (argc > 2) {
        log_error("Too many arguments");
        return APP_INVALID_ARG;
    }

    return app_list_directory((argc == 2) ? argv[1] : NULL);
}

static int32_t app_cmd_cd_handler(int argc, char *argv[])
{
    if (argc != 2) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return app_change_directory(argv[1]);
}

static int32_t app_cmd_mkdir_handler(int argc, char *argv[])
{
    if (argc != 2) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return fat_mkdir(argv[1]);
}

static int32_t app_cmd_rmdir_handler(int argc, char *argv[])
{
    if (argc != 2) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return fat_rmdir(argv[1]);
}

static int32_t app_cmd_cat_handler(int argc, char *argv[])
{
    if (argc != 2) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return app_read_file(argv[1]);
}

static int32_t app_cmd_write_handler(int argc, char *argv[])
{
    if (argc < 3) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    /* Nối các tham số còn lại thành nội dung */
    char content[APP_DATA_BUF_SIZE] = {0};
    for (int i = 2; i < argc; i++) {
        if (i > 2) {
            strcat(content, " ");
        }
        strcat(content, argv[i]);
    }

    return app_write_file(argv[1], content, strlen(content));
}

static int32_t app_cmd_rm_handler(int argc, char *argv[])
{
    if (argc != 2) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return fat_unlink(argv[1]);
}

static int32_t app_cmd_cp_handler(int argc, char *argv[])
{
    if (argc != 3) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    /* Đọc file nguồn */
    fat_file_t src_file;
    if (fat_open(argv[1], FAT_MODE_READ, &src_file) != FAT_SUCCESS) {
        return APP_ERROR;
    }

    /* Tạo file đích */
    fat_file_t dst_file;
    if (fat_open(argv[2], FAT_MODE_WRITE | FAT_MODE_CREATE, &dst_file) != FAT_SUCCESS) {
        fat_close(&src_file);
        return APP_ERROR;
    }

    /* Copy dữ liệu */
    uint32_t bytes_read, bytes_written;
    uint8_t buffer[APP_DATA_BUF_SIZE];
    int32_t status = APP_SUCCESS;

    while ((status = fat_read(&src_file, buffer, sizeof(buffer), &bytes_read)) == FAT_SUCCESS) {
        if (bytes_read == 0) {
            break;
        }

        if (fat_write(&dst_file, buffer, bytes_read, &bytes_written) != FAT_SUCCESS) {
            status = APP_ERROR;
            break;
        }
    }

    fat_close(&src_file);
    fat_close(&dst_file);

    return status;
}

static int32_t app_cmd_mv_handler(int argc, char *argv[])
{
    if (argc != 3) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    /* Copy file */
    int32_t status = app_cmd_cp_handler(argc, argv);
    if (status != APP_SUCCESS) {
        return status;
    }

    /* Xóa file nguồn */
    return fat_unlink(argv[1]);
}

static int32_t app_cmd_exit_handler(int argc, char *argv[])
{
    (void)argc;  /* Unused parameter */
    (void)argv;  /* Unused parameter */

    app_is_running = false;
    return APP_SUCCESS;
}

/*********************************************************************
 * Main Function
 *********************************************************************/

int main(void)
{
    /* Khởi tạo ứng dụng */
    if (app_init() != APP_SUCCESS) {
        log_error("Failed to initialize application");
        return -1;
    }

    /* Chạy ứng dụng */
    if (app_run() != APP_SUCCESS) {
        log_error("Application error");
        return -1;
    }

    return 0;
}

/*********************************************************************
 * UUID: 5b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/