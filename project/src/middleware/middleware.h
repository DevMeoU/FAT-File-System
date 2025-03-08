/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Middleware cung cấp lớp trung gian giữa tầng ứng dụng và
 *   tầng driver, xử lý các yêu cầu từ ứng dụng và chuyển đổi thành
 *   các lệnh phù hợp cho driver.
 *********************************************************************/
#ifndef __MIDDLEWARE_H
#define __MIDDLEWARE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "common_type.h"
#include "fat_driver.h"
#include "ip_driver.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Buffer Size */
#define MID_BUFFER_SIZE     4096    /* Kích thước buffer */

/* Command Type */
#define MID_CMD_READ        0x01    /* Lệnh đọc */
#define MID_CMD_WRITE       0x02    /* Lệnh ghi */
#define MID_CMD_DELETE      0x03    /* Lệnh xóa */
#define MID_CMD_CREATE      0x04    /* Lệnh tạo */
#define MID_CMD_SEND        0x05    /* Lệnh gửi */
#define MID_CMD_RECEIVE     0x06    /* Lệnh nhận */

/* Status Code */
#define MID_SUCCESS         STATUS_SUCCESS    /* Thành công */
#define MID_ERROR          STATUS_ERROR      /* Lỗi chung */
#define MID_NO_MEMORY      STATUS_NO_MEMORY  /* Không đủ bộ nhớ */
#define MID_INVALID        STATUS_INVALID    /* Tham số không hợp lệ */
#define MID_NOT_FOUND      STATUS_NOT_FOUND  /* Không tìm thấy */
#define MID_TIMEOUT        -10               /* Hết thời gian chờ */
#define MID_BUSY           -11               /* Đang bận */

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Command Structure */
typedef struct {
    uint8_t type;              /* Loại lệnh */
    uint8_t *data;            /* Dữ liệu */
    uint32_t size;            /* Kích thước dữ liệu */
    uint32_t timeout;         /* Thời gian chờ */
} mid_cmd_t;

/* Response Structure */
typedef struct {
    int32_t status;           /* Trạng thái */
    uint8_t *data;            /* Dữ liệu */
    uint32_t size;            /* Kích thước dữ liệu */
} mid_resp_t;

/* Callback Function Type */
typedef void (*mid_callback_t)(mid_resp_t *resp);

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo middleware
 * 
 * @return MID_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t mid_init(void);

/**
 * @brief Xử lý lệnh đồng bộ
 * 
 * @param cmd Con trỏ đến lệnh
 * @param resp Con trỏ đến response
 * @return MID_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t mid_process_sync(const mid_cmd_t *cmd, mid_resp_t *resp);

/**
 * @brief Xử lý lệnh bất đồng bộ
 * 
 * @param cmd Con trỏ đến lệnh
 * @param callback Hàm callback
 * @return MID_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t mid_process_async(const mid_cmd_t *cmd, mid_callback_t callback);

/**
 * @brief Đọc dữ liệu từ file
 * 
 * @param path Đường dẫn file
 * @param buffer Buffer lưu dữ liệu
 * @param size Kích thước buffer
 * @param bytes_read Con trỏ đến số byte đã đọc
 * @return MID_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t mid_read_file(const char *path, void *buffer, uint32_t size, uint32_t *bytes_read);

/**
 * @brief Ghi dữ liệu vào file
 * 
 * @param path Đường dẫn file
 * @param buffer Buffer chứa dữ liệu
 * @param size Kích thước dữ liệu
 * @param bytes_written Con trỏ đến số byte đã ghi
 * @return MID_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t mid_write_file(const char *path, const void *buffer, uint32_t size, uint32_t *bytes_written);

/**
 * @brief Gửi dữ liệu qua mạng
 * 
 * @param data Con trỏ đến dữ liệu
 * @param size Kích thước dữ liệu
 * @param timeout Thời gian chờ (ms)
 * @return MID_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t mid_send_data(const void *data, uint32_t size, uint32_t timeout);

/**
 * @brief Nhận dữ liệu từ mạng
 * 
 * @param buffer Buffer lưu dữ liệu
 * @param size Kích thước buffer
 * @param bytes_received Con trỏ đến số byte đã nhận
 * @param timeout Thời gian chờ (ms)
 * @return MID_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t mid_receive_data(void *buffer, uint32_t size, uint32_t *bytes_received, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif /* __MIDDLEWARE_H */

/*********************************************************************
 * UUID: 7b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
