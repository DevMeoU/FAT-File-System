/**
 * @file print_color.h
 * @brief Header file for print_color.c
 * @author Le Duc Son
 * @date 2022-04-20
 * @updated 2025-03-19
 */

#ifndef PRINT_COLOR_H
#define PRINT_COLOR_H

/**
 * ANSI color codes
 */
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
#define COLOR_ITALIC  "\033[3m"
#define COLOR_UNDERLINE "\033[4m"

/**
 * Print a formatted string with a specified color
 * @param color ANSI color code
 * @param format Format string
 * @param ... Additional arguments
 */
void print_color(const char* color, const char* format, ...);

/**
 * Print an error message in red color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_error(const char* format, ...);

/**
 * Print a success message in green color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_success(const char* format, ...);

/**
 * Print a warning message in yellow color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_warning(const char* format, ...);

/**
 * Print an info message in blue color
 * @param format Format string
 * @param ... Additional arguments
 */
void print_info(const char* format, ...);

#endif // PRINT_COLOR_H
