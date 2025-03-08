/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Print Color cung cấp các hàm để in màu ra terminal.
 *********************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include "print_color.h"

void log_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\033[32m[INFO] ");  /* Màu xanh lá */
    vprintf(format, args);
    printf("\033[0m\n");  /* Reset màu */
    va_end(args);
}

void log_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\033[31m[ERROR] ");  /* Màu đỏ */
    vprintf(format, args);
    printf("\033[0m\n");  /* Reset màu */
    va_end(args);
}

void log_warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\033[33m[WARNING] ");  /* Màu vàng */
    vprintf(format, args);
    printf("\033[0m\n");  /* Reset màu */
    va_end(args);
}

void log_debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("\033[36m[DEBUG] ");  /* Màu xanh dương */
    vprintf(format, args);
    printf("\033[0m\n");  /* Reset màu */
    va_end(args);
}

/*********************************************************************
 * UUID: 6b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 