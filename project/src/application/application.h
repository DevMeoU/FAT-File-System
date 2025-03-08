/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Application cung cấp giao diện người dùng để tương tác với
 *   hệ thống FAT, cho phép người dùng thực hiện các thao tác như xem
 *   danh sách file, đọc/ghi file, tạo/xóa thư mục.
 *********************************************************************/
#ifndef __APPLICATION_H
#define __APPLICATION_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "../common/common_types.h"
#include "../middleware/middleware.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Buffer sizes */
#define APP_PATH_BUF_SIZE   256    /* Kích thước buffer đường dẫn */
#define APP_CMD_BUF_SIZE    256    /* Kích thước buffer lệnh */
#define APP_DATA_BUF_SIZE   4096   /* Kích thước buffer dữ liệu */
#define APP_MAX_ARGS        16     /* Số lượng tham số tối đa */

/* Command Code */
#define APP_CMD_HELP        "help"    /* Hiển thị trợ giúp */
#define APP_CMD_LIST        "ls"      /* Liệt kê thư mục */
#define APP_CMD_CHANGE_DIR  "cd"      /* Đổi thư mục */
#define APP_CMD_MAKE_DIR    "mkdir"   /* Tạo thư mục */
#define APP_CMD_REMOVE_DIR  "rmdir"   /* Xóa thư mục */
#define APP_CMD_READ        "cat"     /* Đọc file */
#define APP_CMD_WRITE       "write"   /* Ghi file */
#define APP_CMD_DELETE      "rm"      /* Xóa file */
#define APP_CMD_COPY        "cp"      /* Sao chép file */
#define APP_CMD_MOVE        "mv"      /* Di chuyển file */
#define APP_CMD_EXIT        "exit"    /* Thoát chương trình */

/* Command Description */
#define APP_CMD_HELP_DESC        "Hiển thị thông tin trợ giúp"
#define APP_CMD_LIST_DESC        "Liệt kê nội dung thư mục"
#define APP_CMD_CHANGE_DIR_DESC  "Thay đổi thư mục hiện tại"
#define APP_CMD_MAKE_DIR_DESC    "Tạo thư mục mới"
#define APP_CMD_REMOVE_DIR_DESC  "Xóa thư mục rỗng"
#define APP_CMD_READ_DESC        "Hiển thị nội dung file"
#define APP_CMD_WRITE_DESC       "Ghi nội dung vào file"
#define APP_CMD_DELETE_DESC      "Xóa file"
#define APP_CMD_COPY_DESC        "Sao chép file"
#define APP_CMD_MOVE_DESC        "Di chuyển/đổi tên file"
#define APP_CMD_EXIT_DESC        "Thoát chương trình"

/* Command Usage */
#define APP_CMD_HELP_USAGE        "help [detail]"
#define APP_CMD_LIST_USAGE        "ls [path]"
#define APP_CMD_CHANGE_DIR_USAGE  "cd <path>"
#define APP_CMD_MAKE_DIR_USAGE    "mkdir <path>"
#define APP_CMD_REMOVE_DIR_USAGE  "rmdir <path>"
#define APP_CMD_READ_USAGE        "cat <file>"
#define APP_CMD_WRITE_USAGE       "write <file> <content>"
#define APP_CMD_DELETE_USAGE      "rm <file>"
#define APP_CMD_COPY_USAGE        "cp <src> <dst>"
#define APP_CMD_MOVE_USAGE        "mv <src> <dst>"
#define APP_CMD_EXIT_USAGE        "exit"

/* Status Code */
#define APP_SUCCESS         STATUS_SUCCESS    /* Thành công */
#define APP_ERROR          STATUS_ERROR      /* Lỗi chung */
#define APP_NO_MEMORY      STATUS_NO_MEMORY  /* Không đủ bộ nhớ */
#define APP_INVALID        STATUS_INVALID    /* Tham số không hợp lệ */
#define APP_NOT_FOUND      STATUS_NOT_FOUND  /* Không tìm thấy */
#define APP_INVALID_CMD    -10               /* Lệnh không hợp lệ */
#define APP_INVALID_ARG    -11               /* Tham số không hợp lệ */

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Command Structure */
typedef struct {
    const char *name;       /* Tên lệnh */
    const char *desc;       /* Mô tả lệnh */
    const char *usage;      /* Cách sử dụng */
    int32_t (*handler)(int argc, char *argv[]);  /* Hàm xử lý */
} app_cmd_t;

/* File Info Structure */
typedef struct {
    char name[256];        /* Tên file */
    uint32_t size;         /* Kích thước */
    uint8_t type;          /* Loại (file/thư mục) */
    uint32_t date;         /* Ngày tạo/sửa */
    uint32_t time;         /* Thời gian tạo/sửa */
} app_file_info_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo ứng dụng
 * 
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_init(void);

/**
 * @brief Chạy ứng dụng
 * 
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_run(void);

/**
 * @brief Xử lý lệnh
 * 
 * @param cmd_line Chuỗi lệnh cần xử lý
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_process_command(const char *cmd_line);

/**
 * @brief Hiển thị trợ giúp
 * 
 * @param cmd Lệnh cần hiển thị trợ giúp, NULL để hiển thị tất cả
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_show_help(const char *cmd);

/**
 * @brief Liệt kê nội dung thư mục
 * 
 * @param path Đường dẫn thư mục, NULL để liệt kê thư mục hiện tại
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_list_directory(const char *path);

/**
 * @brief Đổi thư mục làm việc
 * 
 * @param path Đường dẫn thư mục mới
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_change_directory(const char *path);

/**
 * @brief Đọc nội dung file
 * 
 * @param path Đường dẫn file
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_read_file(const char *path);

/**
 * @brief Ghi nội dung vào file
 * 
 * @param path Đường dẫn file
 * @param data Con trỏ đến dữ liệu
 * @param size Kích thước dữ liệu
 * @return APP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t app_write_file(const char *path, const void *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* __APPLICATION_H */

/*********************************************************************
 * UUID: 9b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/

