/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Common Type định nghĩa các kiểu dữ liệu và mã trạng thái
 *   dùng chung cho toàn bộ dự án.
 *********************************************************************/
#ifndef __COMMON_TYPE_H
#define __COMMON_TYPE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Status Code */
typedef enum {
    STATUS_SUCCESS = 0,     /* Thành công */
    STATUS_ERROR = -1,      /* Lỗi chung */
    STATUS_TIMEOUT = -2,    /* Hết thời gian chờ */
    STATUS_NO_MEMORY = -3,  /* Không đủ bộ nhớ */
    STATUS_BUSY = -4,       /* Đang bận */
    STATUS_INVALID = -5,    /* Tham số không hợp lệ */
    STATUS_NOT_FOUND = -6,  /* Không tìm thấy */
    STATUS_NOT_READY = -7,  /* Chưa sẵn sàng */
    STATUS_FULL = -8,       /* Đã đầy */
    STATUS_EMPTY = -9,      /* Rỗng */
} status_t;

/* Result Type */
typedef struct {
    status_t status;        /* Mã trạng thái */
    union {
        int32_t value;      /* Giá trị số */
        void *ptr;          /* Con trỏ */
        uint8_t data[8];    /* Dữ liệu tùy ý */
    } data;                 /* Dữ liệu kết quả */
} result_t;

/* Version Type */
typedef struct {
    uint8_t major;          /* Phiên bản chính */
    uint8_t minor;          /* Phiên bản phụ */
    uint8_t patch;          /* Bản vá */
    uint8_t build;          /* Số build */
} version_t;

/* Range Type */
typedef struct {
    int32_t min;           /* Giá trị nhỏ nhất */
    int32_t max;           /* Giá trị lớn nhất */
} range_t;

/* Size Type */
typedef struct {
    uint32_t width;        /* Chiều rộng */
    uint32_t height;       /* Chiều cao */
} dimension_t;

/* Point Type */
typedef struct {
    int32_t x;            /* Tọa độ x */
    int32_t y;            /* Tọa độ y */
} point_t;

/* Rectangle Type */
typedef struct {
    point_t position;     /* Vị trí */
    dimension_t size;     /* Kích thước */
} rect_t;

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_TYPE_H */

/*********************************************************************
 * UUID: 6e9d2f1b-3c4a-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
