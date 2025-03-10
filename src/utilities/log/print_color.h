/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File header định nghĩa các hàm in màu ra terminal.
 *********************************************************************/
#ifndef PRINT_COLOR_H
#define PRINT_COLOR_H

#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Definitions
 *********************************************************************/
typedef enum {
    COLOR_BLACK = 30,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
} color_t;

/*********************************************************************
 * Màu sắc
 *********************************************************************/
#define COLOR_RESET "\033[0m"
#define COLOR_BACKGROUND_BLACK "\033[40m"
#define COLOR_BACKGROUND_RED "\033[41m"
#define COLOR_BACKGROUND_GREEN "\033[42m"
#define COLOR_BACKGROUND_YELLOW "\033[43m"
#define COLOR_BACKGROUND_BLUE "\033[44m"
#define COLOR_BACKGROUND_MAGENTA "\033[45m"
#define COLOR_BACKGROUND_CYAN "\033[46m"

/*********************************************************************
 * Function Prototypes
 *********************************************************************/

/**
 * @brief In text với màu và nền
 * 
 * @param format Chuỗi định dạng
 * @param fg_color Màu sắc (ví dụ: COLOR_GREEN cho xanh lá)
 * @param bg_color Nền (ví dụ: COLOR_BACKGROUND_BLACK cho đen)
 * @param ... Các tham số
 */
void print_text(const char *format, color_t fg_color, const char *bg_color, ...);

/**
 * @brief In thông tin với màu xanh lá
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void print_info(const char *format, ...);

/**
 * @brief In lỗi với màu đỏ
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void print_error(const char *format, ...);

/**
 * @brief In cảnh báo với màu vàng
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void print_warning(const char *format, ...);

/**
 * @brief In debug với màu xanh dương
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void print_debug(const char *format, ...);

/**
 * @brief In thành công với màu xanh lá
 * 
 * @param format Chuỗi định dạng
 * @param ... Các tham số
 */
void print_success(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* PRINT_COLOR_H */

/*********************************************************************
 * UUID: 7b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/

