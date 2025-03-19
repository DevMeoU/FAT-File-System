/**
 * @file print_color.c
 * @brief Implementation of functions to print colored messages
 * @date 2022-04-20
 * @updated 2025-03-19
 * @updated by Le Duc Son
 */

#include "print_color.h"
#include <stdio.h>
#include <stdarg.h>

/**
 * Print a formatted string with a specified color
 * @param color ANSI color code
 * @param format Format string
 * @param ... Additional arguments
 */
void print_color(const char* color, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s", color);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

/**
 * Print an error message in red color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[ERROR] ", COLOR_RED);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

/**
 * Print a success message in green color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_success(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[SUCCESS] ", COLOR_GREEN);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

/**
 * Print a warning message in yellow color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[WARNING] ", COLOR_YELLOW);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

/**
 * Print an info message in blue color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[INFO] ", COLOR_BLUE);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}
