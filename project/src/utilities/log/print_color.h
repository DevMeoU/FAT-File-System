/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File header định nghĩa các hàm in màu ra terminal.
 *********************************************************************/
#ifndef __PRINT_COLOR_H
#define __PRINT_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Function Prototypes
 *********************************************************************/

/**
 * @brief In thông tin với màu xanh lá
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void log_info(const char *format, ...);

/**
 * @brief In lỗi với màu đỏ
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void log_error(const char *format, ...);

/**
 * @brief In cảnh báo với màu vàng
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void log_warning(const char *format, ...);

/**
 * @brief In debug với màu xanh dương
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void log_debug(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __PRINT_COLOR_H */

/*********************************************************************
 * UUID: 7b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/

