/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Print Color cung cấp các macro để in màu trên terminal,
 *   hỗ trợ việc hiển thị log với các màu sắc khác nhau để dễ phân biệt.
 *********************************************************************/
#ifndef __PRINT_COLOR_H
#define __PRINT_COLOR_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Text Colors */
#define COLOR_BLACK     "\033[0;30m"
#define COLOR_RED       "\033[0;31m"
#define COLOR_GREEN     "\033[0;32m"
#define COLOR_YELLOW    "\033[0;33m"
#define COLOR_BLUE      "\033[0;34m"
#define COLOR_PURPLE    "\033[0;35m"
#define COLOR_CYAN      "\033[0;36m"
#define COLOR_WHITE     "\033[0;37m"

/* Bold Text Colors */
#define COLOR_BOLD_BLACK   "\033[1;30m"
#define COLOR_BOLD_RED     "\033[1;31m"
#define COLOR_BOLD_GREEN   "\033[1;32m"
#define COLOR_BOLD_YELLOW  "\033[1;33m"
#define COLOR_BOLD_BLUE    "\033[1;34m"
#define COLOR_BOLD_PURPLE  "\033[1;35m"
#define COLOR_BOLD_CYAN    "\033[1;36m"
#define COLOR_BOLD_WHITE   "\033[1;37m"

/* Background Colors */
#define COLOR_BG_BLACK     "\033[40m"
#define COLOR_BG_RED       "\033[41m"
#define COLOR_BG_GREEN     "\033[42m"
#define COLOR_BG_YELLOW    "\033[43m"
#define COLOR_BG_BLUE      "\033[44m"
#define COLOR_BG_PURPLE    "\033[45m"
#define COLOR_BG_CYAN      "\033[46m"
#define COLOR_BG_WHITE     "\033[47m"

/* Reset Color */
#define COLOR_RESET     "\033[0m"

/* Log Level Colors */
#define COLOR_DEBUG     COLOR_CYAN
#define COLOR_INFO      COLOR_GREEN
#define COLOR_WARNING   COLOR_YELLOW
#define COLOR_ERROR     COLOR_RED
#define COLOR_FATAL     COLOR_BOLD_RED

/*********************************************************************
 * Function Macros
 *********************************************************************/

/**
 * @brief In chuỗi với màu chỉ định
 * 
 * @param color Mã màu
 * @param fmt Chuỗi định dạng
 * @param ... Các tham số
 */
#define print_color(color, fmt, ...) \
    printf(color fmt COLOR_RESET, ##__VA_ARGS__)

/**
 * @brief In log debug
 * 
 * @param fmt Chuỗi định dạng
 * @param ... Các tham số
 */
#define log_debug(fmt, ...) \
    print_color(COLOR_DEBUG, "[DEBUG] " fmt "\n", ##__VA_ARGS__)

/**
 * @brief In log info
 * 
 * @param fmt Chuỗi định dạng
 * @param ... Các tham số
 */
#define log_info(fmt, ...) \
    print_color(COLOR_INFO, "[INFO] " fmt "\n", ##__VA_ARGS__)

/**
 * @brief In log warning
 * 
 * @param fmt Chuỗi định dạng
 * @param ... Các tham số
 */
#define log_warning(fmt, ...) \
    print_color(COLOR_WARNING, "[WARNING] " fmt "\n", ##__VA_ARGS__)

/**
 * @brief In log error
 * 
 * @param fmt Chuỗi định dạng
 * @param ... Các tham số
 */
#define log_error(fmt, ...) \
    print_color(COLOR_ERROR, "[ERROR] " fmt "\n", ##__VA_ARGS__)

/**
 * @brief In log fatal
 * 
 * @param fmt Chuỗi định dạng
 * @param ... Các tham số
 */
#define log_fatal(fmt, ...) \
    print_color(COLOR_FATAL, "[FATAL] " fmt "\n", ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __PRINT_COLOR_H */

/*********************************************************************
 * UUID: 5d7c1e9a-2b4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/

