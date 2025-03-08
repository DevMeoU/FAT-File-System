/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module HAL (Hardware Abstraction Layer) cung cấp interface để tương tác
 *   với phần cứng, cho phép truy cập các tài nguyên phần cứng một cách
 *   độc lập với nền tảng cụ thể.
 *********************************************************************/
#ifndef __HAL_H
#define __HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Status codes */
#define HAL_SUCCESS            0x00
#define HAL_ERROR             -1
#define HAL_TIMEOUT           -2
#define HAL_INVALID_PARAM     -3
#define HAL_BUSY             -4
#define HAL_NOT_READY        -5

/* Buffer sizes */
#define HAL_MAX_BUFFER_SIZE   1024
#define HAL_MIN_BUFFER_SIZE   16

/* Timeouts */
#define HAL_DEFAULT_TIMEOUT   1000  /* ms */
#define HAL_MIN_TIMEOUT      10    /* ms */
#define HAL_MAX_TIMEOUT      10000 /* ms */

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* HAL Configuration */
typedef struct {
    uint32_t clock_speed;     /* Tốc độ clock (Hz) */
    uint32_t buffer_size;     /* Kích thước buffer */
    uint32_t timeout;         /* Timeout mặc định (ms) */
    bool interrupt_enable;    /* Cho phép ngắt */
} hal_config_t;

/* Device Information */
typedef struct {
    uint32_t device_id;       /* ID thiết bị */
    uint32_t manufacturer_id; /* ID nhà sản xuất */
    uint32_t version;        /* Phiên bản */
    uint32_t capabilities;   /* Khả năng */
} hal_device_info_t;

/* Transfer Mode */
typedef enum {
    HAL_MODE_POLLING = 0,    /* Chế độ polling */
    HAL_MODE_INTERRUPT,      /* Chế độ ngắt */
    HAL_MODE_DMA            /* Chế độ DMA */
} hal_transfer_mode_t;

/* Device Status */
typedef struct {
    bool is_initialized;     /* Đã khởi tạo */
    bool is_busy;           /* Đang bận */
    uint32_t error_count;   /* Số lỗi */
    uint32_t transfer_count;/* Số lần truyền */
} hal_status_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo HAL
 *
 * @param config Cấu hình HAL
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_init(const hal_config_t *config);

/**
 * @brief Đọc dữ liệu từ thiết bị
 *
 * @param buffer Buffer lưu dữ liệu
 * @param size Số byte cần đọc
 * @param timeout Thời gian timeout (ms)
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_read(void *buffer, uint32_t size, uint32_t timeout);

/**
 * @brief Ghi dữ liệu vào thiết bị
 *
 * @param buffer Buffer chứa dữ liệu
 * @param size Số byte cần ghi
 * @param timeout Thời gian timeout (ms)
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_write(const void *buffer, uint32_t size, uint32_t timeout);

/**
 * @brief Cấu hình chế độ truyền
 *
 * @param mode Chế độ truyền
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_set_transfer_mode(hal_transfer_mode_t mode);

/**
 * @brief Lấy thông tin thiết bị
 *
 * @param info Con trỏ đến struct lưu thông tin
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_get_device_info(hal_device_info_t *info);

/**
 * @brief Lấy trạng thái thiết bị
 *
 * @param status Con trỏ đến struct lưu trạng thái
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_get_status(hal_status_t *status);

/**
 * @brief Reset thiết bị
 *
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_reset(void);

/**
 * @brief Đăng ký callback cho sự kiện
 *
 * @param callback Hàm callback
 * @param event_id ID sự kiện
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t hal_register_callback(void (*callback)(void *), uint32_t event_id);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_H */

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/