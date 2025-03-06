/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#ifndef PRINT_COLOR_H
#define PRINT_COLOR_H

/*********************************************************************
 * Include
 *********************************************************************/
#include <stdio.h>
#include <string.h>

/*********************************************************************
 * Define
 *********************************************************************/

// Định nghĩa macro cho mã màu văn bản ANSI
#define ANSI_COLOR_BLACK   "\033[30m"
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_BLUE    "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN    "\033[36m"
#define ANSI_COLOR_WHITE   "\033[37m"
#define ANSI_COLOR_RESET   "\033[0m"

// Định nghĩa macro cho mã màu nền ANSI
#define ANSI_BG_BLACK   "\033[40m"
#define ANSI_BG_RED     "\033[41m"
#define ANSI_BG_GREEN   "\033[42m"
#define ANSI_BG_YELLOW  "\033[43m"
#define ANSI_BG_BLUE    "\033[44m"
#define ANSI_BG_MAGENTA "\033[45m"
#define ANSI_BG_CYAN    "\033[46m"
#define ANSI_BG_WHITE   "\033[47m"

#define PRINT_COLOR(color, fmt, ...) \
    printf(color fmt ANSI_COLOR_RESET, ##__VA_ARGS__)

/*********************************************************************
 * Static function
 *********************************************************************/
// Hàm in văn bản với màu và nền
static inline void print_colored(const char *text, const char *color, const char *bg_color) {
    printf("%s%s%s%s", color, bg_color, text, ANSI_COLOR_RESET);
}
/*********************************************************************
 * Function prototypes
 *********************************************************************/

#endif /* PRINT_COLOR_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/

