/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module log cung cấp các macro và hàm để ghi log debug và theo dõi
 *   hoạt động của hệ thống.
 *********************************************************************/
#ifndef __MODULE_LOG_H
#define __MODULE_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Log levels */
#define LOG_LEVEL_NONE      0
#define LOG_LEVEL_ERROR     1
#define LOG_LEVEL_WARNING   2
#define LOG_LEVEL_INFO      3
#define LOG_LEVEL_DEBUG     4

/* Current log level */
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

/* Log colors */
#define LOG_COLOR_RED     "\x1b[31m"
#define LOG_COLOR_GREEN   "\x1b[32m"
#define LOG_COLOR_YELLOW  "\x1b[33m"
#define LOG_COLOR_BLUE    "\x1b[34m"
#define LOG_COLOR_RESET   "\x1b[0m"

/* Log macros */
#if LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(fmt, ...) \
    printf(LOG_COLOR_RED "[ERROR] %s:%d: " fmt LOG_COLOR_RESET "\n", \
           __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_ERROR(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARNING
#define LOG_WARNING(fmt, ...) \
    printf(LOG_COLOR_YELLOW "[WARNING] %s:%d: " fmt LOG_COLOR_RESET "\n", \
           __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_WARNING(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(fmt, ...) \
    printf(LOG_COLOR_GREEN "[INFO] " fmt LOG_COLOR_RESET "\n", ##__VA_ARGS__)
#else
#define LOG_INFO(fmt, ...)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(fmt, ...) \
    printf(LOG_COLOR_BLUE "[DEBUG] %s:%d: " fmt LOG_COLOR_RESET "\n", \
           __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_DEBUG(fmt, ...)
#endif

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Log configuration */
typedef struct {
    uint8_t level;
    bool color_enabled;
    bool timestamp_enabled;
    const char *output_file;
} log_config_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo module log
 *
 * @param config Cấu hình log
 * @return 0 nếu thành công, -1 nếu thất bại
 */
int32_t log_init(const log_config_t *config);

/**
 * @brief Ghi log với level và message tùy chọn
 *
 * @param level Log level
 * @param fmt Format string
 * @param ... Các tham số cho format string
 */
void log_write(uint8_t level, const char *fmt, ...);

/**
 * @brief Đóng module log và giải phóng tài nguyên
 */
void log_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_LOG_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/ 