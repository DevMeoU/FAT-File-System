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
#include "application.h"
#include "print_color.h"

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
        .desc = "Hiển thị trợ giúp",
        .usage = "help [command]",
        .handler = app_cmd_help_handler
    },
    {
        .name = APP_CMD_LIST,
        .desc = "Liệt kê thư mục",
        .usage = "ls [path]",
        .handler = app_cmd_list_handler
    },
    {
        .name = APP_CMD_CHANGE_DIR,
        .desc = "Đổi thư mục",
        .usage = "cd <path>",
        .handler = app_cmd_cd_handler
    },
    {
        .name = APP_CMD_MAKE_DIR,
        .desc = "Tạo thư mục",
        .usage = "mkdir <path>",
        .handler = app_cmd_mkdir_handler
    },
    {
        .name = APP_CMD_REMOVE_DIR,
        .desc = "Xóa thư mục",
        .usage = "rmdir <path>",
        .handler = app_cmd_rmdir_handler
    },
    {
        .name = APP_CMD_READ,
        .desc = "Đọc file",
        .usage = "cat <file>",
        .handler = app_cmd_cat_handler
    },
    {
        .name = APP_CMD_WRITE,
        .desc = "Ghi file",
        .usage = "write <file> <data>",
        .handler = app_cmd_write_handler
    },
    {
        .name = APP_CMD_DELETE,
        .desc = "Xóa file",
        .usage = "rm <file>",
        .handler = app_cmd_rm_handler
    },
    {
        .name = APP_CMD_COPY,
        .desc = "Sao chép file",
        .usage = "cp <src> <dst>",
        .handler = app_cmd_cp_handler
    },
    {
        .name = APP_CMD_MOVE,
        .desc = "Di chuyển file",
        .usage = "mv <src> <dst>",
        .handler = app_cmd_mv_handler
    },
    {
        .name = APP_CMD_EXIT,
        .desc = "Thoát chương trình",
        .usage = "exit",
        .handler = app_cmd_exit_handler
    }
};

/* Running flag */
static bool app_is_running = false;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Xử lý lệnh help
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_help_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh ls
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_list_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh cd
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_cd_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh mkdir
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_mkdir_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh rmdir
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_rmdir_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh cat
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_cat_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh write
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_write_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh rm
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_rm_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh cp
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_cp_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh mv
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_mv_handler(int argc, char *argv[]);

/**
 * @brief Xử lý lệnh exit
 * 
 * @param argc Số lượng tham số
 * @param argv Mảng tham số
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t app_cmd_exit_handler(int argc, char *argv[]);

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
    for (int i = 0; i < sizeof(app_cmd_table)/sizeof(app_cmd_t); i++) {
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
        for (int i = 0; i < sizeof(app_cmd_table)/sizeof(app_cmd_t); i++) {
            printf("  %-10s - %s\n", app_cmd_table[i].name, app_cmd_table[i].desc);
        }
    } else {
        /* Tìm lệnh trong bảng */
        for (int i = 0; i < sizeof(app_cmd_table)/sizeof(app_cmd_t); i++) {
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
    fat_file_info_t info;
    if (fat_stat(path, &info) != FAT_SUCCESS) {
        return APP_NOT_FOUND;
    }

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

    /* Đọc file */
    uint32_t bytes_read;
    if (mid_read_file(path, app_data_buffer, sizeof(app_data_buffer), &bytes_read) != MID_SUCCESS) {
        return APP_ERROR;
    }

    /* In nội dung */
    printf("%.*s", bytes_read, app_data_buffer);
    if (bytes_read > 0 && app_data_buffer[bytes_read - 1] != '\n') {
        printf("\n");
    }

    return APP_SUCCESS;
}

int32_t app_write_file(const char *path, const void *data, uint32_t size)
{
    if (path == NULL || data == NULL) {
        return APP_INVALID;
    }

    /* Ghi file */
    uint32_t bytes_written;
    if (mid_write_file(path, data, size, &bytes_written) != MID_SUCCESS) {
        return APP_ERROR;
    }

    return APP_SUCCESS;
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

    return (fat_mkdir(argv[1]) == FAT_SUCCESS) ? APP_SUCCESS : APP_ERROR;
}

static int32_t app_cmd_rmdir_handler(int argc, char *argv[])
{
    if (argc != 2) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return (fat_rmdir(argv[1]) == FAT_SUCCESS) ? APP_SUCCESS : APP_ERROR;
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
    if (argc != 3) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return app_write_file(argv[1], argv[2], strlen(argv[2]));
}

static int32_t app_cmd_rm_handler(int argc, char *argv[])
{
    if (argc != 2) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    return (fat_unlink(argv[1]) == FAT_SUCCESS) ? APP_SUCCESS : APP_ERROR;
}

static int32_t app_cmd_cp_handler(int argc, char *argv[])
{
    if (argc != 3) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    /* Đọc file nguồn */
    uint32_t bytes_read;
    if (mid_read_file(argv[1], app_data_buffer, sizeof(app_data_buffer), &bytes_read) != MID_SUCCESS) {
        return APP_ERROR;
    }

    /* Ghi file đích */
    return app_write_file(argv[2], app_data_buffer, bytes_read);
}

static int32_t app_cmd_mv_handler(int argc, char *argv[])
{
    if (argc != 3) {
        log_error("Invalid number of arguments");
        return APP_INVALID_ARG;
    }

    /* Copy file */
    if (app_cmd_cp_handler(argc, argv) != APP_SUCCESS) {
        return APP_ERROR;
    }

    /* Xóa file nguồn */
    return app_cmd_rm_handler(2, argv);
}

static int32_t app_cmd_exit_handler(int argc, char *argv[])
{
    if (argc != 1) {
        log_error("Too many arguments");
        return APP_INVALID_ARG;
    }

    app_is_running = false;
    return APP_SUCCESS;
}

/*********************************************************************
 * UUID: ab8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/