/**
 * @file common_types.h
 * @author Le Duc Son (son.leduc92@gmail.com)
 * @date 2019-07-25
 * @brief This file contains common types for whole system
 */

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/**
 * Defines common types for whole system
 */

/**
 * File system mode
 */
typedef enum {
    MODE_READ_ONLY,
    MODE_READ_WRITE
} FileSystemMode;

/**
 * FAT type
 */
typedef enum {
    FAT_TYPE_12,
    FAT_TYPE_16,
    FAT_TYPE_32,
    FAT_TYPE_UNKNOWN
} FatType;

/**
 * Sector size
 */
typedef enum {
    SECTOR_SIZE_512 = 512,
    SECTOR_SIZE_1024 = 1024,
    SECTOR_SIZE_2048 = 2048,
    SECTOR_SIZE_4096 = 4096
} SectorSize;

/**
 * Cache size
 */
typedef enum {
    CACHE_SIZE_16 = 16,
    CACHE_SIZE_32 = 32,
    CACHE_SIZE_64 = 64,
    CACHE_SIZE_128 = 128
} CacheSize;

/**
 * Directory name length
 */
typedef enum {
    DIR_NAME_LEN_8 = 8,
    DIR_NAME_LEN_16 = 16,
    DIR_NAME_LEN_32 = 32,
    DIR_NAME_LEN_64 = 64
} DirNameLength;

/**
 * Max length of file name
 */
#define FILE_NAME_LEN 255
#define FILE_NAME_MAX 255

/**
 * File system configuration
 */
typedef struct {
    const char * img_path;
    FileSystemMode mode;
    FatType fat_type;
    SectorSize sector_size;
    CacheSize cache_size;
    DirNameLength dir_name_len;
} FileSystemConfig;

/**
 * Date time
 */
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} DateTime;

/**
 * File type
 */
typedef enum {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_VOLUME_ID,
    FILE_TYPE_UNKNOWN
} FileType;

/**
 * File attributes
 */
typedef struct {
    bool read_only;
    bool hidden;
    bool system;
    bool volume_id;
    bool directory;
    bool archive;
} FileAttributes;

#endif // COMMON_TYPES_H

