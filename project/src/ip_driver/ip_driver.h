#ifndef IP_DRIVER_H
#define IP_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "../common/common_types.h"

typedef struct {
    FILE* img_file;
    uint32_t buffer_size;
} IPDriver;

/**
 * Khởi tạo IP Driver
 * @param driver Con trỏ đến cấu trúc IPDriver
 * @param img_path Đường dẫn đến file ảnh
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int ip_driver_init(IPDriver* driver, const char* img_path);

/**
 * Đọc một sector từ file ảnh
 * @param driver Con trỏ đến cấu trúc IPDriver
 * @param offset Số thứ tự sector cần đọc
 * @param buffer Buffer để lưu dữ liệu đọc được
 * @return Số byte đọc được nếu thành công, -1 nếu thất bại
 */
int ip_driver_read_sector(IPDriver* driver, uint32_t offset, void* buffer);

/**
 * Ghi một sector vào file ảnh
 * @param driver Con trỏ đến cấu trúc IPDriver
 * @param offset Số thứ tự sector cần ghi
 * @param buffer Buffer chứa dữ liệu cần ghi
 * @return Số byte ghi được nếu thành công, -1 nếu thất bại
 */
int ip_driver_write_sector(IPDriver* driver, uint32_t offset, const void* buffer);

/**
 * Đóng IP Driver
 * @param driver Con trỏ đến cấu trúc IPDriver
 */
void ip_driver_close(IPDriver* driver);

#endif
