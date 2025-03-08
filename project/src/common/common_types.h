/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Định nghĩa các kiểu dữ liệu và mã trạng thái chung được sử dụng
 *   xuyên suốt trong hệ thống.
 *********************************************************************/
#ifndef __COMMON_TYPES_H
#define __COMMON_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*********************************************************************
 * Common Status Codes
 *********************************************************************/

/* Success codes */
#define STATUS_SUCCESS          0    /* Thành công */

/* Error codes */
#define STATUS_ERROR           -1    /* Lỗi chung */
#define STATUS_INVALID         -2    /* Tham số không hợp lệ */
#define STATUS_NOT_FOUND       -3    /* Không tìm thấy */
#define STATUS_EXISTS          -4    /* Đã tồn tại */
#define STATUS_DISK_FULL       -5    /* Đĩa đầy */
#define STATUS_READ_ONLY       -6    /* Chỉ đọc */
#define STATUS_EOF             -7    /* Hết tập tin */
#define STATUS_INVALID_NAME    -8    /* Tên không hợp lệ */
#define STATUS_ROOT_FULL       -9    /* Thư mục gốc đầy */
#define STATUS_INVALID_PATH    -10   /* Đường dẫn không hợp lệ */
#define STATUS_READ_FAILED     -11   /* Đọc thất bại */
#define STATUS_WRITE_FAILED    -12   /* Ghi thất bại */
#define STATUS_INVALID_PARAMETER -13  /* Tham số không hợp lệ */
#define STATUS_NO_MEMORY       -14   /* Không đủ bộ nhớ */
#define STATUS_NO_SPACE        -15   /* Không đủ không gian */

/* System status codes */
#define STATUS_NOT_READY       -20   /* Chưa sẵn sàng */
#define STATUS_BUSY           -21   /* Đang bận */
#define STATUS_TIMEOUT        -22   /* Hết thời gian chờ */
#define STATUS_OVERFLOW       -23   /* Tràn bộ đệm */
#define STATUS_CRC_ERROR      -24   /* Lỗi CRC */

/* Warning codes */
#define STATUS_WARNING        -30   /* Cảnh báo chung */
#define STATUS_WOULD_BLOCK    -31   /* Sẽ bị block */
#define STATUS_DEPRECATED     -32   /* Đã lỗi thời */
#define STATUS_PARTIAL        -33   /* Thành công một phần */
#define STATUS_PENDING        -34   /* Đang chờ xử lý */

/*********************************************************************
 * Common Type Definitions
 *********************************************************************/

/* Version information */
typedef struct {
    uint8_t major;    /* Major version number */
    uint8_t minor;    /* Minor version number */
    uint8_t patch;    /* Patch level */
    uint8_t reserved; /* Reserved for future use */
} version_info_t;

/* Buffer descriptor */
typedef struct {
    void *data;      /* Pointer to data */
    uint32_t size;   /* Buffer size */
    uint32_t used;   /* Number of bytes used */
} buffer_t;

/* Time value */
typedef struct {
    uint16_t year;   /* Year (1970-2099) */
    uint8_t month;   /* Month (1-12) */
    uint8_t day;     /* Day (1-31) */
    uint8_t hour;    /* Hour (0-23) */
    uint8_t minute;  /* Minute (0-59) */
    uint8_t second;  /* Second (0-59) */
    uint16_t ms;     /* Milliseconds (0-999) */
} fatfs_time_t;

/* Callback function type */
typedef void (*callback_t)(void *param);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_TYPES_H */ 