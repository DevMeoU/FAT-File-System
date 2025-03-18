#ifndef PRINT_COLOR_H
#define PRINT_COLOR_H

// Định nghĩa các mã màu ANSI
#define COLOR_RESET   "\033[0m"
#define COLOR_BLACK   "\033[30m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_WHITE   "\033[37m"

#define COLOR_BOLD    "\033[1m"
#define COLOR_ITALIC "\033[3m"
#define COLOR_UNDERLINE "\033[4m"

/**
 * In chuỗi với màu chỉ định
 * @param color Mã màu ANSI
 * @param format Chuỗi định dạng
 * @param ... Các tham số bổ sung
 */
void print_color(const char* color, const char* format, ...);

/**
 * In thông báo lỗi (màu đỏ)
 * @param format Chuỗi định dạng
 * @param ... Các tham số bổ sung
 */
void print_error(const char* format, ...);

/**
 * In thông báo thành công (màu xanh lá)
 * @param format Chuỗi định dạng
 * @param ... Các tham số bổ sung
 */
void print_success(const char* format, ...);

/**
 * In thông báo cảnh báo (màu vàng)
 * @param format Chuỗi định dạng
 * @param ... Các tham số bổ sung
 */
void print_warning(const char* format, ...);

/**
 * In thông báo thông tin (màu xanh dương)
 * @param format Chuỗi định dạng
 * @param ... Các tham số bổ sung
 */
void print_info(const char* format, ...);

#endif // PRINT_COLOR_H
