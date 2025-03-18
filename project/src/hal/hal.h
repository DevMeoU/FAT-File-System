#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include "../common/common_types.h"
#include "../ip_driver/ip_driver.h"

typedef struct {
    IPDriver ip_driver;
    SectorSize sector_size;
} HAL;

/**
 * Khởi tạo HAL
 * @param hal Con trỏ đến cấu trúc HAL
 * @param img_path Đường dẫn đến file ảnh
 * @param sector_size Kích thước sector
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int hal_init(HAL* hal, const char* img_path, SectorSize sector_size);

/** 
 * Hủy bỏ HAL
 * @param hal Con trỏ đến cấu trúc HAL
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int hal_deinit(HAL* hal);

/**
 * Đọc một sector từ file ảnh
 * @param hal Con trỏ đến cấu trúc HAL
 * @param sector_number Số thứ tự sector cần đọc
 * @param buffer Buffer để lưu dữ liệu đọc được
 * @return Số byte đọc được nếu thành công, -1 nếu thất bại
 */
int hal_read_sector(HAL* hal, uint32_t sector_number, void* buffer);

/**
 * Ghi một sector vào file ảnh
 * @param hal Con trỏ đến cấu trúc HAL
 * @param sector_number Số thứ tự sector cần ghi
 * @param buffer Buffer chứa dữ liệu cần ghi
 * @return Số byte ghi được nếu thành công, -1 nếu thất bại
 */
int hal_write_sector(HAL* hal, uint32_t sector_number, const void* buffer);

/**
 * Đóng HAL
 * @param hal Con trỏ đến cấu trúc HAL
 */
void hal_close(HAL* hal);

/**
 * Lấy kích thước sector
 * @param hal Con trỏ đến cấu trúc HAL
 * @return Kích thước sector
 */
uint32_t hal_get_sector_size(HAL* hal);

#endif // HAL_H
