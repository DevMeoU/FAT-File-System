#ifndef IP_DRIVER_H
#define IP_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "../common/common_types.h"

typedef struct {
    FILE* img_file;
    uint32_t sector_size;
} IPDriver;

/**
 * Khởi tạo IP Driver
 * @param driver Con trỏ đến cấu trúc IPDriver
 * @param img_path Đường dẫn đến file ảnh
 * @param sector_size Kích thước sector (512, 1024, 2048, 4096)
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int ip_driver_init(IPDriver* driver, const char* img_path, uint32_t sector_size);

/**
 * Đọc một sector từ file ảnh
 * @param driver Con trỏ đến cấu trúc IPDriver
 * @param sector_number Số thứ tự sector cần đọc
 * @param buffer Buffer để lưu dữ liệu đọc được
 * @return Số byte đọc được nếu thành công, -1 nếu thất bại
 */
int ip_driver_read_sector(IPDriver* driver, uint32_t sector_number, void* buffer);

/**
 * Ghi một sector vào file ảnh
 * @param driver Con trỏ đến cấu trúc IPDriver
 * @param sector_number Số thứ tự sector cần ghi
 * @param buffer Buffer chứa dữ liệu cần ghi
 * @return Số byte ghi được nếu thành công, -1 nếu thất bại
 */
int ip_driver_write_sector(IPDriver* driver, uint32_t sector_number, const void* buffer);

/**
 * Đóng IP Driver
 * @param driver Con trỏ đến cấu trúc IPDriver
 */
void ip_driver_close(IPDriver* driver);

#endif
