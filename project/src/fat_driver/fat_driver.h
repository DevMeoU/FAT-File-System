/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module FAT Driver cung cấp các hàm để đọc/ghi dữ liệu trên hệ thống
 *   tập tin FAT (File Allocation Table). Hỗ trợ các phiên bản FAT12,
 *   FAT16 và FAT32.
 *********************************************************************/
#ifndef __FAT_DRIVER_H
#define __FAT_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "common_type.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* FAT Type */
#define FAT_TYPE_12          12  /* FAT12 */
#define FAT_TYPE_16          16  /* FAT16 */
#define FAT_TYPE_32          32  /* FAT32 */

/* Sector Size */
#define FAT_SECTOR_SIZE      512 /* Kích thước sector */

/* File Attributes */
#define FAT_ATTR_READ_ONLY   0x01  /* Chỉ đọc */
#define FAT_ATTR_HIDDEN      0x02  /* Ẩn */
#define FAT_ATTR_SYSTEM      0x04  /* Hệ thống */
#define FAT_ATTR_VOLUME_ID   0x08  /* Nhãn ổ đĩa */
#define FAT_ATTR_DIRECTORY   0x10  /* Thư mục */
#define FAT_ATTR_ARCHIVE     0x20  /* Lưu trữ */
#define FAT_ATTR_LONG_NAME   0x0F  /* Tên dài */

/* File Access Mode */
#define FAT_MODE_READ        0x01  /* Đọc */
#define FAT_MODE_WRITE       0x02  /* Ghi */
#define FAT_MODE_CREATE      0x04  /* Tạo mới */
#define FAT_MODE_APPEND      0x08  /* Thêm vào cuối */
#define FAT_MODE_TRUNCATE    0x10  /* Cắt ngắn */

/* Error Codes */
#define FAT_SUCCESS          STATUS_SUCCESS    /* Thành công */
#define FAT_ERROR           STATUS_ERROR      /* Lỗi chung */
#define FAT_NO_MEMORY       STATUS_NO_MEMORY  /* Không đủ bộ nhớ */
#define FAT_INVALID         STATUS_INVALID    /* Tham số không hợp lệ */
#define FAT_NOT_FOUND       STATUS_NOT_FOUND  /* Không tìm thấy */
#define FAT_EXISTS          -10               /* Đã tồn tại */
#define FAT_READ_ONLY       -11               /* Chỉ đọc */
#define FAT_DISK_FULL       -12               /* Đĩa đầy */
#define FAT_ROOT_FULL       -13               /* Thư mục gốc đầy */
#define FAT_EOF             -14               /* Hết tập tin */
#define FAT_INVALID_NAME    -15               /* Tên không hợp lệ */
#define FAT_INVALID_PATH    -16               /* Đường dẫn không hợp lệ */

/*********************************************************************
 * Type Definitions  
 *********************************************************************/

/* FAT Boot Sector */
typedef struct {
    uint8_t  jump_boot[3];        /* Mã nhảy khởi động */
    uint8_t  oem_name[8];         /* Tên OEM */
    uint16_t bytes_per_sector;    /* Số byte mỗi sector */
    uint8_t  sectors_per_cluster; /* Số sector mỗi cluster */
    uint16_t reserved_sectors;    /* Số sector dự trữ */
    uint8_t  number_of_fats;      /* Số bảng FAT */
    uint16_t root_entries;        /* Số entry thư mục gốc */
    uint16_t total_sectors_16;    /* Tổng số sector (16-bit) */
    uint8_t  media;               /* Loại thiết bị */
    uint16_t fat_size_16;         /* Kích thước FAT (FAT12/16) */
    uint16_t sectors_per_track;   /* Số sector mỗi track */
    uint16_t number_of_heads;     /* Số đầu đọc */
    uint32_t hidden_sectors;      /* Số sector ẩn */
    uint32_t total_sectors_32;    /* Tổng số sector (32-bit) */
    union {
        struct {
            uint8_t  drive_number;      /* Số ổ đĩa */
            uint8_t  reserved1;         /* Dự trữ */
            uint8_t  boot_signature;    /* Chữ ký khởi động */
            uint32_t volume_id;         /* ID ổ đĩa */
            uint8_t  volume_label[11];  /* Nhãn ổ đĩa */
            uint8_t  file_system[8];    /* Tên hệ thống tập tin */
        } fat16;
        struct {
            uint32_t fat_size_32;       /* Kích thước FAT (FAT32) */
            uint16_t ext_flags;         /* Cờ mở rộng */
            uint16_t fs_version;        /* Phiên bản */
            uint32_t root_cluster;      /* Cluster thư mục gốc */
            uint16_t fs_info;          /* Sector thông tin */
            uint16_t backup_boot;       /* Sector khởi động dự phòng */
            uint8_t  reserved[12];      /* Dự trữ */
            uint8_t  drive_number;      /* Số ổ đĩa */
            uint8_t  reserved1;         /* Dự trữ */
            uint8_t  boot_signature;    /* Chữ ký khởi động */
            uint32_t volume_id;         /* ID ổ đĩa */
            uint8_t  volume_label[11];  /* Nhãn ổ đĩa */
            uint8_t  file_system[8];    /* Tên hệ thống tập tin */
        } fat32;
    } type;
} fat_boot_sector_t;

/* FAT Directory Entry */
typedef struct {
    uint8_t  name[11];           /* Tên tập tin */
    uint8_t  attributes;         /* Thuộc tính */
    uint8_t  reserved;           /* Dự trữ */
    uint8_t  creation_time_ms;   /* Mili giây tạo */
    uint16_t creation_time;      /* Thời gian tạo */
    uint16_t creation_date;      /* Ngày tạo */
    uint16_t last_access_date;   /* Ngày truy cập */
    uint16_t first_cluster_hi;   /* Cluster đầu (high) */
    uint16_t last_write_time;    /* Thời gian sửa */
    uint16_t last_write_date;    /* Ngày sửa */
    uint16_t first_cluster_lo;   /* Cluster đầu (low) */
    uint32_t file_size;          /* Kích thước tập tin */
} fat_dir_entry_t;

/* FAT File Info */
typedef struct {
    uint8_t  name[256];         /* Tên đầy đủ */
    uint8_t  attributes;        /* Thuộc tính */
    uint32_t size;             /* Kích thước */
    uint32_t cluster;          /* Cluster đầu */
    uint16_t date;             /* Ngày */
    uint16_t time;             /* Thời gian */
} fat_file_info_t;

/* FAT File Handle */
typedef struct {
    fat_file_info_t info;      /* Thông tin tập tin */
    uint32_t position;         /* Vị trí đọc/ghi */
    uint32_t cluster;          /* Cluster hiện tại */
    uint32_t sector;           /* Sector hiện tại */
    uint32_t offset;           /* Offset trong sector */
    uint8_t  mode;            /* Chế độ truy cập */
    bool     modified;        /* Đã sửa đổi */
} fat_file_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo FAT driver
 * 
 * @param boot_sector Con trỏ đến boot sector
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_init(const fat_boot_sector_t *boot_sector);

/**
 * @brief Mở tập tin
 * 
 * @param path Đường dẫn tập tin
 * @param mode Chế độ truy cập
 * @param file Con trỏ đến handle tập tin
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_open(const char *path, uint8_t mode, fat_file_t *file);

/**
 * @brief Đóng tập tin
 * 
 * @param file Handle tập tin
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_close(fat_file_t *file);

/**
 * @brief Đọc dữ liệu từ tập tin
 * 
 * @param file Handle tập tin
 * @param buffer Buffer lưu dữ liệu
 * @param size Số byte cần đọc
 * @param bytes_read Con trỏ đến số byte đã đọc
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_read(fat_file_t *file, void *buffer, uint32_t size, uint32_t *bytes_read);

/**
 * @brief Ghi dữ liệu vào tập tin
 * 
 * @param file Handle tập tin
 * @param buffer Buffer chứa dữ liệu
 * @param size Số byte cần ghi
 * @param bytes_written Con trỏ đến số byte đã ghi
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_write(fat_file_t *file, const void *buffer, uint32_t size, uint32_t *bytes_written);

/**
 * @brief Di chuyển con trỏ đọc/ghi
 * 
 * @param file Handle tập tin
 * @param offset Offset cần di chuyển
 * @param origin Vị trí bắt đầu (SEEK_SET, SEEK_CUR, SEEK_END)
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_seek(fat_file_t *file, int32_t offset, int32_t origin);

/**
 * @brief Lấy thông tin tập tin
 * 
 * @param path Đường dẫn tập tin
 * @param info Con trỏ đến thông tin tập tin
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_stat(const char *path, fat_file_info_t *info);

/**
 * @brief Xóa tập tin
 * 
 * @param path Đường dẫn tập tin
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_unlink(const char *path);

/**
 * @brief Tạo thư mục
 * 
 * @param path Đường dẫn thư mục
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_mkdir(const char *path);

/**
 * @brief Xóa thư mục
 * 
 * @param path Đường dẫn thư mục
 * @return FAT_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t fat_rmdir(const char *path);

#ifdef __cplusplus
}
#endif

#endif /* __FAT_DRIVER_H */

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
