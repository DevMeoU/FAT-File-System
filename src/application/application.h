/*********************************************************************
 * Module Application - Định nghĩa giao diện người dùng
 *********************************************************************/
#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdint.h>
#include "../common/common_types.h"

/* Error codes */
#define APP_SUCCESS 0
#define APP_ERROR -1

/* Access modes */
#define MODE_READ_ONLY "ro"
#define MODE_READ_WRITE "rw"

/* Loại FAT */
#define FAT_TYPE_12 12
#define FAT_TYPE_16 16
#define FAT_TYPE_32 32

/* Kích thước sector */
#define SECTOR_SIZE_512  512
#define SECTOR_SIZE_1024 1024
#define SECTOR_SIZE_2048 2048
#define SECTOR_SIZE_4096 4096

/* Kích thước cache */
#define CACHE_SIZE_16  16
#define CACHE_SIZE_32  32
#define CACHE_SIZE_64  64
#define CACHE_SIZE_128 128

/* Độ dài tên thư mục */
#define DIR_NAME_LEN_8  8
#define DIR_NAME_LEN_16 16
#define DIR_NAME_LEN_32 32
#define DIR_NAME_LEN_64 64

/* Độ dài tên file */
#define FILE_NAME_LEN 255
#define FILE_NAME_MAX 255

/* Application configuration */
typedef struct {
    const char* mode;           // Access mode
    uint32_t sector_size;      // Sector size
    uint32_t cache_size;       // Cache size
    uint32_t dir_name_len;     // Directory name length
} AppConfig;

/* Application context */
typedef struct {
    int running;               // Running state
    char* img_path;           // Image file path
    char* current_path;       // Current directory path
    int is_root_mode;         // Root/user mode flag
    AppConfig config;         // Configuration
} Application;

/* Public functions */
int application_init(Application* app, const char* img_path, const char* mode);
void application_run_shell(Application* app);
void application_cleanup(Application* app);

#endif /* APPLICATION_H */

/*********************************************************************
 * UUID: 9b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/

