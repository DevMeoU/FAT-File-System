/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Interface cho Storage Driver, định nghĩa các hàm cần thiết để
 *   tương tác với thiết bị lưu trữ.
 *********************************************************************/
#ifndef __STORAGE_DRIVER_H
#define __STORAGE_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "common_types.h"

/* Status codes */
#define STORAGE_SUCCESS     0
#define STORAGE_ERROR      -1
#define STORAGE_TIMEOUT    -2
#define STORAGE_NO_MEDIA   -3

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Đọc một sector từ thiết bị lưu trữ
 * 
 * @param sector Số thứ tự sector
 * @param buffer Buffer lưu dữ liệu đọc được
 * @return STATUS_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t storage_read_sector(uint32_t sector, uint8_t *buffer);

/**
 * @brief Ghi một sector vào thiết bị lưu trữ
 * 
 * @param sector Số thứ tự sector
 * @param buffer Buffer chứa dữ liệu cần ghi
 * @return STATUS_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t storage_write_sector(uint32_t sector, const uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_DRIVER_H */ 