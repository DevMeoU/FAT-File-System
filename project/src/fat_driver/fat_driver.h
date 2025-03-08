/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Interface cho FAT File System module, cung cấp các hàm để quản lý
 *   và truy cập hệ thống tập tin FAT (File Allocation Table). Hỗ trợ
 *   các phiên bản FAT12, FAT16 và FAT32.
 *********************************************************************/
#ifndef __FAT_FS_H
#define __FAT_FS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../common/common_types.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Status codes */
#define FAT_SUCCESS             0x00
#define FAT_ERROR              -1
#define FAT_INVALID            -2
#define FAT_NOT_FOUND          -3
#define FAT_EXISTS             -4
#define FAT_READ_ONLY          -5
#define FAT_DISK_FULL          -6
#define FAT_EOF                -7

/* FAT types */
#define FAT_TYPE_12            12
#define FAT_TYPE_16            16
#define FAT_TYPE_32            32

/* File attributes */
#define FAT_ATTR_READ_ONLY     0x01
#define FAT_ATTR_HIDDEN        0x02
#define FAT_ATTR_SYSTEM        0x04
#define FAT_ATTR_VOLUME_ID     0x08
#define FAT_ATTR_DIRECTORY     0x10
#define FAT_ATTR_ARCHIVE       0x20
#define FAT_ATTR_LONG_NAME     0x0F

/* Access modes */
#define FAT_MODE_READ          0x01
#define FAT_MODE_WRITE         0x02
#define FAT_MODE_CREATE        0x04
#define FAT_MODE_APPEND        0x08
#define FAT_MODE_TRUNCATE      0x10

/* Seek origins */
#define FAT_SEEK_SET          0
#define FAT_SEEK_CUR          1
#define FAT_SEEK_END          2

/* Configuration flags */
#define FAT_CONFIG_READ_ONLY   0x01
#define FAT_CONFIG_NO_CACHE    0x02
#define FAT_CONFIG_SYNC_WRITE  0x04

/* Buffer sizes */
#define FAT_MAX_PATH          256
#define FAT_MAX_NAME          255
#define FAT_SECTOR_SIZE       512

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* FAT Configuration */
typedef struct {
    uint8_t type;           /* FAT type (12/16/32) */
    uint32_t sector_size;   /* Sector size in bytes */
    uint32_t cluster_size;  /* Cluster size in sectors */
    uint32_t root_entries;  /* Number of root directory entries */
    uint32_t total_sectors; /* Total number of sectors */
    uint32_t flags;        /* Configuration flags */
} fat_config_t;

/* File Information */
typedef struct {
    char name[FAT_MAX_NAME];  /* File name */
    uint8_t attributes;       /* File attributes */
    uint32_t size;           /* File size in bytes */
    uint32_t cluster;        /* First cluster number */
    uint16_t date;           /* Last modified date */
    uint16_t time;           /* Last modified time */
} fat_file_info_t;

/* Directory Entry */
typedef struct __attribute__((packed)) {
    uint8_t name[11];         /* 8.3 name */
    uint8_t attributes;       /* File attributes */
    uint8_t reserved;         /* Reserved for Windows NT */
    uint8_t creation_time_ms; /* Creation time, milliseconds */
    uint16_t creation_time;   /* Creation time */
    uint16_t creation_date;   /* Creation date */
    uint16_t last_access;     /* Last access date */
    uint16_t first_cluster_hi;/* High word of first cluster number */
    uint16_t last_write_time; /* Last write time */
    uint16_t last_write_date; /* Last write date */
    uint16_t first_cluster_lo;/* Low word of first cluster number */
    uint32_t file_size;       /* File size in bytes */
} fat_dir_entry_t;

/* File Handle */
typedef struct {
    fat_file_info_t info;    /* File information */
    uint32_t position;       /* Current position */
    uint32_t cluster;        /* Current cluster */
    uint32_t sector;         /* Current sector */
    uint32_t offset;         /* Offset in current sector */
    uint8_t mode;           /* Access mode */
    bool modified;          /* Modified flag */
} fat_file_t;

/* Boot Sector */
typedef struct __attribute__((packed)) {
    uint8_t jump[3];          /* Boot strap short or near jump */
    uint8_t oem_name[8];      /* OEM name and version */
    uint16_t bytes_per_sector;/* Bytes per sector */
    uint8_t sectors_per_cluster; /* Sectors per cluster */
    uint16_t reserved_sectors;/* Reserved sectors count */
    uint8_t fats;            /* Number of FATs */
    uint16_t root_entries;    /* Number of root directory entries */
    uint16_t total_sectors_16;/* Total sectors (16-bit) */
    uint8_t media;           /* Media type */
    uint16_t fat_size_16;    /* FAT size in sectors */
    uint16_t sectors_per_track;/* Sectors per track */
    uint16_t heads;          /* Number of heads */
    uint32_t hidden_sectors; /* Hidden sectors */
    uint32_t total_sectors_32;/* Total sectors (32-bit) */
    union {
        struct {
            uint8_t drive_number;    /* BIOS drive number */
            uint8_t reserved1;       /* Reserved */
            uint8_t boot_signature;  /* Boot signature */
            uint32_t volume_id;      /* Volume ID */
            uint8_t volume_label[11];/* Volume label */
            uint8_t fs_type[8];      /* File system type */
        } __attribute__((packed)) fat16;
        struct {
            uint32_t fat_size_32;    /* FAT size in sectors */
            uint16_t ext_flags;      /* Extended flags */
            uint16_t fs_version;     /* File system version */
            uint32_t root_cluster;   /* First cluster of root directory */
            uint16_t fs_info;        /* Sector number of FSINFO structure */
            uint16_t backup_boot;    /* Sector number of backup boot sector */
            uint8_t reserved[12];    /* Reserved */
            uint8_t drive_number;    /* BIOS drive number */
            uint8_t reserved1;       /* Reserved */
            uint8_t boot_signature;  /* Boot signature */
            uint32_t volume_id;      /* Volume ID */
            uint8_t volume_label[11];/* Volume label */
            uint8_t fs_type[8];      /* File system type */
        } __attribute__((packed)) fat32;
    };
    uint16_t signature;       /* Signature */
} fat_boot_sector_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo FAT file system
 * 
 * @param config Con trỏ đến cấu hình
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_init(const fat_config_t *config);

/**
 * @brief Mở một tập tin
 * 
 * @param path Đường dẫn tập tin
 * @param mode Chế độ truy cập
 * @param file Con trỏ đến file handle
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file);

/**
 * @brief Đóng một tập tin
 * 
 * @param file Con trỏ đến file handle
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_close(fat_file_t *file);

/**
 * @brief Đọc dữ liệu từ tập tin
 * 
 * @param file Con trỏ đến file handle
 * @param buffer Buffer lưu dữ liệu
 * @param size Số byte cần đọc
 * @param bytes_read Con trỏ đến biến lưu số byte đã đọc
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read);

/**
 * @brief Ghi dữ liệu vào tập tin
 * 
 * @param file Con trỏ đến file handle
 * @param buffer Buffer chứa dữ liệu
 * @param size Số byte cần ghi
 * @param bytes_written Con trỏ đến biến lưu số byte đã ghi
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written);

/**
 * @brief Di chuyển con trỏ đọc/ghi
 * 
 * @param file Con trỏ đến file handle
 * @param offset Offset cần di chuyển
 * @param origin Điểm bắt đầu di chuyển
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin);

/**
 * @brief Lấy thông tin tập tin
 * 
 * @param path Đường dẫn tập tin
 * @param info Con trỏ đến struct lưu thông tin
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_stat(const char *path, fat_file_info_t *info);

/**
 * @brief Xóa một tập tin
 * 
 * @param path Đường dẫn tập tin
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_unlink(const char *path);

/**
 * @brief Tạo một thư mục
 * 
 * @param path Đường dẫn thư mục
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_mkdir(const char *path);

/**
 * @brief Xóa một thư mục
 * 
 * @param path Đường dẫn thư mục
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_rmdir(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* __FAT_FS_H */

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
