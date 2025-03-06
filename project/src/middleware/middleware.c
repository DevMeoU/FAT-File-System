/*********************************************************************
 * Include
 *********************************************************************/
#include "middleware.h"
#include "fat_driver.h"

/*********************************************************************
 * Define
 *********************************************************************/

/*********************************************************************
 * Function prototypes
 *********************************************************************/

/*********************************************************************
 * Implementations
 *********************************************************************/
/**
 * @brief Initialize middleware and FAT driver
 * @param img_path Path to the floppy image file
 * @return 0 on success, -1 on failure
 */
int middleware_init(const char *img_path) {
    if (fat_driver_init(img_path) != 0) {
        printf("Failed to initialize FAT driver.\n");
        return -1;
    }
    return 0;
}

/**
 * @brief List directory contents
 * @param path Path of the directory to list
 */
void middleware_list_directory(const char *path) {
    // printf("Listing files/directories at %s:\n", path);
    // Gọi FAT Driver để liệt kê thư mục tại path
    if (fat_driver_list_directory(path) != 0) {
        printf("Failed to list directory.\n");
    }
    // Giả sử fat_driver_list_directory đã xử lý việc hiển thị danh sách
}

/**
 * @brief Change current directory
 * @param path New directory path
 * @param current_path Current path to update
 */
void middleware_change_directory(const char *path, char *current_path) {
    // Kiểm tra xem thư mục có tồn tại không
    if (fat_driver_directory_exists(path)) {
        strcpy(current_path, path);
        printf("Đã chuyển đến thư mục: %s\n", path);
    } else {
        printf("Thư mục không tồn tại: %s\n", path);
    }
}

/**
 * @brief Read and display file content
 * @param filename Path of the file to read
 */
void middleware_read_file(const char *filename) {
    // Gọi FAT Driver để đọc file
    if (fat_driver_read_file(filename) != 0) {
        printf("Failed to read file.\n");
    }
    // Giả sử fat_driver_read_file đã xử lý việc hiển thị nội dung
}
