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

/* Success codes (0x00 - 0x0F) */
#define STATUS_SUCCESS           0x00
#define STATUS_PENDING          0x01
#define STATUS_TIMEOUT          0x02
#define STATUS_PARTIAL          0x03

/* Error codes (0x10 - 0x1F) */
#define STATUS_ERROR            0x10
#define STATUS_INVALID          0x11
#define STATUS_NOT_READY        0x12
#define STATUS_BUSY            0x13
#define STATUS_TIMEOUT_ERROR   0x14
#define STATUS_OVERFLOW        0x15
#define STATUS_CRC_ERROR       0x16
#define STATUS_NOT_FOUND       0x17

/* Warning codes (0x20 - 0x2F) */
#define STATUS_WARNING          0x20
#define STATUS_WOULD_BLOCK      0x21
#define STATUS_DEPRECATED       0x22

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
} time_t;

/* Callback function type */
typedef void (*callback_t)(void *param);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_TYPES_H */ 