#include "print_color.h"
#include <stdio.h>
#include <stdarg.h>

void print_color(const char* color, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s", color);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

void print_error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[ERROR] ", COLOR_RED);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

void print_success(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[SUCCESS] ", COLOR_GREEN);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

void print_warning(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[WARNING] ", COLOR_YELLOW);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}

void print_info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    printf("%s[INFO] ", COLOR_BLUE);
    vprintf(format, args);
    printf("%s", COLOR_RESET);
    
    fflush(stdout);
    va_end(args);
}
