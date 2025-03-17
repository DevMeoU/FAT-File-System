/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Định nghĩa các kiểu dữ liệu và mã trạng thái chung được sử dụng
 *   xuyên suốt trong hệ thống.
 *********************************************************************/
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*********************************************************************
 * Status Codes
 *********************************************************************/
typedef enum {
    STATUS_SUCCESS = 0,
    STATUS_ERROR = -1,
    STATUS_INVALID_PARAMETER = -2,
    STATUS_NO_MEMORY = -3,
    STATUS_NOT_FOUND = -4,
    STATUS_ACCESS_DENIED = -5,
    STATUS_ALREADY_EXISTS = -6,
    STATUS_IO_ERROR = -7,
    STATUS_TIMEOUT = -8,
    STATUS_BUSY = -9,
    STATUS_NOT_READY = -10
} StatusCode;

/*********************************************************************
 * File Attributes
 *********************************************************************/
typedef enum {
    FAT_ATTR_READ_ONLY = 0x01,
    FAT_ATTR_HIDDEN = 0x02,
    FAT_ATTR_SYSTEM = 0x04,
    FAT_ATTR_VOLUME_ID = 0x08,
    FAT_ATTR_DIRECTORY = 0x10,
    FAT_ATTR_ARCHIVE = 0x20,
    FAT_ATTR_LONG_NAME = 0x0F
} FatAttribute;

/*********************************************************************
 * File Access Modes
 *********************************************************************/
typedef enum {
    FAT_MODE_READ = 0x01,
    FAT_MODE_WRITE = 0x02,
    FAT_MODE_APPEND = 0x04,
    FAT_MODE_CREATE = 0x08
} FatAccessMode;

/*********************************************************************
 * FAT Types
 *********************************************************************/
typedef enum {
    FAT_TYPE_12 = 12,
    FAT_TYPE_16 = 16,
    FAT_TYPE_32 = 32
} FatType;

/*********************************************************************
 * Sector Size Configurations
 *********************************************************************/
typedef enum {
    SECTOR_SIZE_512 = 512,
    SECTOR_SIZE_1024 = 1024,
    SECTOR_SIZE_2048 = 2048,
    SECTOR_SIZE_4096 = 4096
} SectorSize;

/*********************************************************************
 * Cache Size Configurations
 *********************************************************************/
typedef enum {
    CACHE_SIZE_16 = 16,
    CACHE_SIZE_32 = 32,
    CACHE_SIZE_64 = 64,
    CACHE_SIZE_128 = 128
} CacheSize;

/*********************************************************************
 * Directory Name Length Configurations
 *********************************************************************/
typedef enum {
    DIR_NAME_LEN_8 = 8,
    DIR_NAME_LEN_16 = 16,
    DIR_NAME_LEN_32 = 32,
    DIR_NAME_LEN_64 = 64
} DirNameLength;

/*********************************************************************
 * System Configuration
 *********************************************************************/
typedef struct {
    FatAccessMode access_mode;
    FatType fat_type;
    SectorSize sector_size;
    CacheSize cache_size;
    DirNameLength dir_name_length;
} fat_config_t;

/*********************************************************************
 * Common Type Definitions
 *********************************************************************/

/* Version information */
typedef struct {
    uint8_t major;    /* Major version number */
    uint8_t minor;    /* Minor version number */
    uint8_t patch;    /* Patch level */
    uint8_t reserved; /* Reserved for future use */
} version_info_t;

/* Buffer descriptor */
typedef struct {
    void *data;      /* Pointer to data */
    uint32_t size;   /* Buffer size */
    uint32_t used;   /* Number of bytes used */
} buffer_t;

/* Time value */
typedef struct {
    uint16_t year;   /* Year (1970-2099) */
    uint8_t month;   /* Month (1-12) */
    uint8_t day;     /* Day (1-31) */
    uint8_t hour;    /* Hour (0-23) */
    uint8_t minute;  /* Minute (0-59) */
    uint8_t second;  /* Second (0-59) */
    uint16_t ms;     /* Milliseconds (0-999) */
} fatfs_time_t;

/* Callback function type */
typedef void (*callback_t)(void *param);

/* FAT File/Directory Entry */
typedef struct {
    char name[256];
    uint8_t attributes;
    uint32_t size;
    uint32_t first_cluster;
    fatfs_time_t create_time;
    fatfs_time_t modify_time;
} fat_entry_t;

/* Application Buffer Sizes */
#define APP_PATH_BUF_SIZE    1024
#define APP_CMD_BUF_SIZE     1024
#define APP_DATA_BUF_SIZE    4096

#ifdef __cplusplus
}
#endif

#endif /* COMMON_TYPES_H */ 