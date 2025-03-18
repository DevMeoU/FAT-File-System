#ifndef FAT_DRIVER_H
#define FAT_DRIVER_H

#include "../common/common_types.h"
#include "../hal/hal.h"
#include "fat_driver_types.h"

/**
 * Khởi tạo FAT Driver
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @param config Cấu hình hệ thống tệp
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int fat_driver_init(FATDriver* driver, const FileSystemConfig config);

/**
 *  Hủy bỏ FAT Driver
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int fat_driver_deinit(FATDriver* driver);

/**
 * Mount hệ thống tệp
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int fat_driver_mount(FATDriver* driver);

/**
 * Unmount hệ thống tệp
 * @param driver Con trỏ đến cấu trúc FATDriver
 */
void fat_driver_unmount(FATDriver* driver);

/**
 * Lấy thư mục gốc
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @return Con trỏ đến thư mục gốc
 */
FileNode* fat_driver_get_root_directory(FATDriver* driver);

/**
 * Lấy thư mục hiện tại
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @return Con trỏ đến thư mục hiện tại
 */
FileNode* fat_driver_get_current_directory(FATDriver* driver);

/**
 * Đặt thư mục hiện tại
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @param directory Con trỏ đến thư mục cần đặt
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int fat_driver_set_current_directory(FATDriver* driver, FileNode* directory);

/**
 * Tìm đường dẫn
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @param path Đường dẫn cần tìm
 * @return Con trỏ đến node nếu tìm thấy, NULL nếu không tìm thấy
 */
FileNode* fat_driver_find_path(FATDriver* driver, const char* path);

/**
 * Tìm đường dẫn đệ quy
 * @param current Con trỏ đến node hiện tại
 * @param path Đường dẫn cần tìm
 * @return Con trỏ đến node nếu tìm thấy, NULL nếu không tìm thấy
 */
FileNode* fat_driver_find_path_recursive(FileNode* current, const char* path);

/**
 * Đọc nội dung file
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @param file Con trỏ đến file cần đọc
 * @param buffer Buffer để lưu dữ liệu đọc được
 * @param size Kích thước cần đọc
 * @return Số byte đã đọc nếu thành công, -1 nếu thất bại
 */
int fat_driver_read_file(FATDriver* driver, FileNode* file, void* buffer, uint32_t size);

/**
 * Ghi nội dung file
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @param file Con trỏ đến file cần ghi
 * @param buffer Buffer chứa dữ liệu cần ghi
 * @param size Kích thước cần ghi
 * @return Số byte đã ghi nếu thành công, -1 nếu thất bại
 */
int fat_driver_write_file(FATDriver* driver, FileNode* file, const void* buffer, uint32_t size);

/**
 * Lấy loại FAT
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @return Loại FAT (FAT12, FAT16, FAT32)
 */
FatType fat_driver_get_fat_type(FATDriver* driver);

/**
 * Lấy thông tin hệ thống tệp
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @param total_size Con trỏ để lưu tổng kích thước
 * @param free_size Con trỏ để lưu kích thước trống
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int fat_driver_get_filesystem_info(FATDriver* driver, uint64_t* total_size, uint64_t* free_size);

/**
 * Chuyển đổi cluster thành sector
 * @param driver Con trỏ đến cấu trúc FATDriver
 * @param cluster Số cluster
 * @return Số sector
 */
uint32_t fat_driver_cluster_to_sector(FATDriver* driver, uint32_t cluster);

/**
 * Giải phóng node file
 * @param node Con trỏ đến node cần giải phóng
 */
void fat_driver_free_file_node(FileNode* node);

#endif // FAT_DRIVER_H
