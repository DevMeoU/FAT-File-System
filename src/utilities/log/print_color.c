/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module Print Color cung cấp các hàm để in màu ra terminal.
 *********************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include "print_color.h"

void print_text(const char *format, color_t fg_color, const char *bg_color, ...)
{
    va_list args;
    
    /* Set text color */
    printf("\033[%d%s", fg_color, bg_color);
    
    /* Print formatted text */
    va_start(args, bg_color);
    vprintf(format, args);
    va_end(args);
    
    /* Reset color */
    printf("%s", COLOR_RESET);
}

void print_info(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("%s[INFO] ", COLOR_GREEN);
    vprintf(format, args);
    printf("%s\n", COLOR_RESET);
    va_end(args);
}

void print_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("%s[ERROR] ", COLOR_RED);
    vprintf(format, args);
    printf("%s\n", COLOR_RESET);
    va_end(args);
}

void print_warning(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("%s[WARNING] ", COLOR_YELLOW);
    vprintf(format, args);
    printf("%s\n", COLOR_RESET);
    va_end(args);
}

void print_debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("%s[DEBUG] ", COLOR_BLUE);
    vprintf(format, args);
    printf("%s\n", COLOR_RESET);
    va_end(args);
}

void print_success(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    printf("%s[SUCCESS] ", COLOR_GREEN);
    vprintf(format, args);
    printf("%s\n", COLOR_RESET);
    va_end(args);
}

/*********************************************************************
 * UUID: 6b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 