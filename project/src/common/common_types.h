#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>

// Định nghĩa các kiểu dữ liệu chung cho toàn bộ hệ thống

// Các mode hỗ trợ
typedef enum {
    MODE_READ_ONLY,
    MODE_READ_WRITE
} FileSystemMode;

// Các loại FAT hỗ trợ
typedef enum {
    FAT_TYPE_12,
    FAT_TYPE_16,
    FAT_TYPE_32
} FatType;

// Các kích thước sector hỗ trợ
typedef enum {
    SECTOR_SIZE_512 = 512,
    SECTOR_SIZE_1024 = 1024,
    SECTOR_SIZE_2048 = 2048,
    SECTOR_SIZE_4096 = 4096
} SectorSize;

// Các kích thước cache hỗ trợ
typedef enum {
    CACHE_SIZE_16 = 16,
    CACHE_SIZE_32 = 32,
    CACHE_SIZE_64 = 64,
    CACHE_SIZE_128 = 128
} CacheSize;

// Các độ dài tên thư mục hỗ trợ
typedef enum {
    DIR_NAME_LEN_8 = 8,
    DIR_NAME_LEN_16 = 16,
    DIR_NAME_LEN_32 = 32,
    DIR_NAME_LEN_64 = 64
} DirNameLength;

// Độ dài tên file và số ký tự tối đa
#define FILE_NAME_LEN 255
#define FILE_NAME_MAX 255

// Cấu trúc cấu hình hệ thống
typedef struct {
    FileSystemMode mode;
    FatType fat_type;
    SectorSize sector_size;
    CacheSize cache_size;
    DirNameLength dir_name_len;
} FileSystemConfig;

// Kiểu dữ liệu cho thời gian
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DateTime;

// Kiểu file
typedef enum {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_VOLUME_ID,
    FILE_TYPE_UNKNOWN
} FileType;

// Thuộc tính file
typedef struct {
    bool read_only;
    bool hidden;
    bool system;
    bool volume_id;
    bool directory;
    bool archive;
} FileAttributes;

#endif // COMMON_TYPES_H
