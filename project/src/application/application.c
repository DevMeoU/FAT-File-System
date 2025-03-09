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
static int32_t app_cmd_mount_handler(int argc, char *argv[]);
static int32_t app_cmd_exit_handler(int argc, char *argv[]);
static int32_t app_cmd_clear_handler(int argc, char *argv[]);

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
static const struct {
    const char *name;
    const char *desc;
    const char *usage;
    int32_t (*handler)(int argc, char *argv[]);
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

/* Running flag */
static bool app_is_running = false;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t app_init(void)
{
    /* Khởi tạo biến */
    memset(app_cmd_buffer, 0, sizeof(app_cmd_buffer));
    memset(app_data_buffer, 0, sizeof(app_data_buffer));
    app_is_running = true;

    /* Hiển thị thông báo chào mừng */
    print_text("\nWelcome to FAT File System Shell!\n", COLOR_GREEN, COLOR_BLACK);
    print_text("Please use 'mount <file>' to mount a file system\n", COLOR_GREEN, COLOR_BLACK);
    print_text("Type 'help' for list of commands\n\n", COLOR_GREEN, COLOR_BLACK);

    return APP_SUCCESS;
}

int32_t app_run(void)
{
    while (app_is_running) {
        /* Hiển thị prompt */
        print_text("FATFS ", COLOR_GREEN, COLOR_BLACK);
        print_text("%s ", COLOR_YELLOW, COLOR_BLACK, app_current_dir);
        print_text("> ", COLOR_MAGENTA, COLOR_BLACK);
        fflush(stdout);

        /* Đọc lệnh */
        if (fgets(app_cmd_buffer, sizeof(app_cmd_buffer), stdin) == NULL) {
            break;
        }

        /* Xử lý lệnh */
        if (app_process_command(app_cmd_buffer) != APP_SUCCESS) {
            log_error("Command failed");
        }
    }

    return APP_SUCCESS;
}

int32_t app_process_command(const char *cmd_line)
{
    char *argv[APP_MAX_ARGS];
    int argc = 0;
    char *token;
    char *cmd_copy;
    size_t cmd_len;

    /* Kiểm tra tham số */
    if (!cmd_line) {
        return APP_INVALID;
    }

    /* Tạo bản sao của lệnh để xử lý */
    cmd_len = strlen(cmd_line) + 1;
    cmd_copy = (char *)malloc(cmd_len);
    if (!cmd_copy) {
        return APP_NO_MEMORY;
    }
    memcpy(cmd_copy, cmd_line, cmd_len);

    /* Tách lệnh thành các tham số */
    token = strtok(cmd_copy, " \t\n\r");
    while (token && argc < APP_MAX_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n\r");
    }

    /* Kiểm tra lệnh rỗng */
    if (argc == 0) {
        free(cmd_copy);
        return APP_SUCCESS;
    }

    /* Tìm và thực thi lệnh */
    for (int i = 0; app_cmd_table[i].name != NULL; i++) {
        if (strcmp(argv[0], app_cmd_table[i].name) == 0) {
            int32_t ret = app_cmd_table[i].handler(argc, argv);
            free(cmd_copy);
            return ret;
        }
    }

    /* Lệnh không hợp lệ */
    log_error("Unknown command: %s", argv[0]);
    free(cmd_copy);
    return APP_INVALID;
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

int32_t app_list_directory(const char *target_path)
{
    fat_file_t dir;
    
    /* Open directory */
    if (fat_open(target_path, FAT_MODE_READ, &dir) != STATUS_SUCCESS) {
        log_error("Could not open directory %s", target_path);
        return STATUS_ERROR;
    }

    /* Đọc các entry */
    fat_dir_entry_t entry;
    uint32_t bytes_read;
    while (fat_read(&dir, &entry, sizeof(entry), &bytes_read) == APP_SUCCESS) {
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
    return STATUS_SUCCESS;
}

int32_t app_change_directory(const char *path)
{
    fat_file_t dir;
    
    /* Open directory */
    if (fat_open(path, FAT_MODE_READ, &dir) != STATUS_SUCCESS) {
        printf("Error: Could not open directory %s\n", path);
        return STATUS_ERROR;
    }

    /* Kiểm tra là thư mục */
    fat_dir_entry_t info;
    if (fat_stat(path, &info) != APP_SUCCESS || !(info.attributes & FAT_ATTR_DIRECTORY)) {
        fat_close(&dir);
        return STATUS_ERROR;
    }

    fat_close(&dir);

    /* Cập nhật đường dẫn hiện tại */
    strncpy(app_current_dir, path, sizeof(app_current_dir) - 1);
    app_current_dir[sizeof(app_current_dir) - 1] = '\0';

    return STATUS_SUCCESS;
}

int32_t app_read_file(const char *path)
{
    fat_file_t file;
    
    /* Open file */
    if (fat_open(path, FAT_MODE_READ, &file) != STATUS_SUCCESS) {
        printf("Error: Could not open file %s\n", path);
        return STATUS_ERROR;
    }

    /* Đọc và hiển thị nội dung */
    uint32_t bytes_read;
    uint8_t buffer[APP_DATA_BUF_SIZE];
    int32_t status = STATUS_SUCCESS;

    while ((status = fat_read(&file, buffer, sizeof(buffer), &bytes_read)) == APP_SUCCESS) {
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
    fat_file_t file;
    
    /* Open file */
    if (fat_open(path, FAT_MODE_WRITE | FAT_MODE_CREATE | FAT_MODE_TRUNCATE, &file) != STATUS_SUCCESS) {
        printf("Error: Could not open file %s\n", path);
        return STATUS_ERROR;
    }

    /* Ghi dữ liệu */
    uint32_t bytes_written;
    int32_t status = fat_write(&file, data, size, &bytes_written);

    fat_close(&file);

    return (status == STATUS_SUCCESS) ? STATUS_SUCCESS : STATUS_ERROR;
}

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t app_cmd_help_handler(int argc, char *argv[])
{
    if (argc > 1) {
        /* Hiển thị trợ giúp cho lệnh cụ thể */
        for (int i = 0; app_cmd_table[i].name != NULL; i++) {
            if (strcmp(argv[1], app_cmd_table[i].name) == 0) {
                printf("\n%s - %s\n", app_cmd_table[i].name, app_cmd_table[i].desc);
                printf("Usage: %s\n\n", app_cmd_table[i].usage);
                return APP_SUCCESS;
            }
        }
        log_error("Unknown command: %s", argv[1]);
        return APP_INVALID;
    }

    /* Hiển thị danh sách lệnh */
    printf("\nAvailable commands:\n");
    for (int i = 0; app_cmd_table[i].name != NULL; i++) {
        printf("  %-10s %s\n", app_cmd_table[i].name, app_cmd_table[i].desc);
    }
    printf("\nType 'help <command>' for more information about a command\n\n");

    return APP_SUCCESS;
}

static int32_t app_cmd_list_handler(int argc, char *argv[])
{
    const char *path = (argc > 1) ? argv[1] : app_current_dir;

    /* Liệt kê thư mục */
    // TODO: Implement directory listing
    (void)path; // Unused parameter
    app_list_directory(path);

    return APP_SUCCESS;
}

static int32_t app_cmd_cd_handler(int argc, char *argv[])
{
    if (argc < 2) {
        log_error("Usage: cd <path>");
        return APP_INVALID;
    }

    /* Thay đổi thư mục */
    // TODO: Implement directory change
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_mkdir_handler(int argc, char *argv[])
{
    if (argc < 2) {
        log_error("Usage: mkdir <path>");
        return APP_INVALID;
    }

    /* Tạo thư mục mới */
    // TODO: Implement directory creation
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_rmdir_handler(int argc, char *argv[])
{
    if (argc < 2) {
        log_error("Usage: rmdir <path>");
        return APP_INVALID;
    }

    /* Xóa thư mục */
    // TODO: Implement directory removal
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_cat_handler(int argc, char *argv[])
{
    if (argc < 2) {
        log_error("Usage: cat <file>");
        return APP_INVALID;
    }

    /* Đọc và hiển thị nội dung file */
    // TODO: Implement file reading
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_write_handler(int argc, char *argv[])
{
    if (argc < 3) {
        log_error("Usage: write <file> <content>");
        return APP_INVALID;
    }

    /* Ghi nội dung vào file */
    // TODO: Implement file writing
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_rm_handler(int argc, char *argv[])
{
    if (argc < 2) {
        log_error("Usage: rm <file>");
        return APP_INVALID;
    }

    /* Xóa file */
    // TODO: Implement file removal
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_cp_handler(int argc, char *argv[])
{
    if (argc < 3) {
        log_error("Usage: cp <source> <destination>");
        return APP_INVALID;
    }

    /* Sao chép file */
    // TODO: Implement file copying
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_mv_handler(int argc, char *argv[])
{
    if (argc < 3) {
        log_error("Usage: mv <source> <destination>");
        return APP_INVALID;
    }

    /* Di chuyển/đổi tên file */
    // TODO: Implement file moving/renaming
    (void)argv; // Unused parameter

    return APP_SUCCESS;
}

static int32_t app_cmd_mount_handler(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: mount <file>\n");
        return APP_INVALID;
    }

    /* Khởi tạo middleware với file đã chỉ định */
    if (mid_init_with_file(argv[1]) != MID_SUCCESS) {
        log_error("Failed to mount file system");
        return APP_ERROR;
    }

    log_info("File system mounted successfully");
    return APP_SUCCESS;
}

static int32_t app_cmd_exit_handler(int argc, char *argv[])
{
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter

    printf("Goodbye!\n");
    exit(0);
}

static int32_t app_cmd_clear_handler(int argc, char *argv[])
{
    (void)argc; // Unused parameter
    (void)argv; // Unused parameter

    system("clear");
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
        return 1;
    }

    /* Chạy ứng dụng */
    if (app_run() != APP_SUCCESS) {
        log_error("Application error");
        return 1;
    }

    return 0;
}

/*********************************************************************
 * UUID: 5b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/