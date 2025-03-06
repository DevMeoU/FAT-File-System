/*
* Middleware Header
* Author: Ducson9112k
*
* Description:
*   Định nghĩa các hàm trung gian để khởi tạo hệ thống, liệt kê thư mục, thay đổi thư mục và đọc file.
*/

#ifndef MIDDLEWARE_H
#define MIDDLEWARE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

/**
 * @brief Initialize middleware and FAT driver.
 *
 * @param img_path Path to the floppy image file.
 * @return 0 on success, -1 on failure.
 */
int middleware_init(const char *img_path);

/**
 * @brief List directory contents.
 *
 * @param path Path of the directory to list.
 */
void middleware_list_directory(const char *path);

/**
 * @brief Change current directory.
 *
 * @param path New directory path.
 * @param current_path Current path to update.
 */
void middleware_change_directory(const char *path, char *current_path);

/**
 * @brief Read and display file content.
 *
 * @param filename Full path of the file to read.
 */
void middleware_read_file(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* MIDDLEWARE_H */
