/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Interface chung cho các storage driver, định nghĩa các hàm và cấu trúc
 *   dữ liệu chuẩn mà mọi storage driver phải tuân thủ để có thể tích hợp
 *   vào hệ thống.
 *********************************************************************/
#ifndef __STORAGE_INTERFACE_H
#define __STORAGE_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "common_types.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Status codes */
#define STORAGE_SUCCESS       STATUS_SUCCESS
#define STORAGE_ERROR        STATUS_ERROR
#define STORAGE_INVALID      STATUS_INVALID
#define STORAGE_TIMEOUT      STATUS_TIMEOUT
#define STORAGE_NOT_READY    STATUS_NOT_READY

/* Storage types */
#define STORAGE_TYPE_BLOCK    0x01  /* Block device (e.g. disk) */
#define STORAGE_TYPE_IP       0x02  /* IP-based storage */
#define STORAGE_TYPE_FLASH    0x03  /* Flash memory */
#define STORAGE_TYPE_RAM      0x04  /* RAM disk */

/* Storage capabilities */
#define STORAGE_CAP_READ      0x01  /* Hỗ trợ đọc */
#define STORAGE_CAP_WRITE     0x02  /* Hỗ trợ ghi */
#define STORAGE_CAP_ERASE     0x04  /* Hỗ trợ xóa */
#define STORAGE_CAP_TRIM      0x08  /* Hỗ trợ TRIM */
#define STORAGE_CAP_CACHE     0x10  /* Hỗ trợ cache */
#define STORAGE_CAP_DMA       0x20  /* Hỗ trợ DMA */

/* Storage limits */
#define STORAGE_MAX_SECTOR_SIZE   4096
#define STORAGE_MIN_SECTOR_SIZE   512
#define STORAGE_MAX_SECTORS       0xFFFFFFFF

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Storage Configuration */
typedef struct {
    uint8_t type;           /* Loại storage */
    uint32_t sector_size;   /* Kích thước sector */
    uint32_t sector_count;  /* Số lượng sector */
    uint32_t capabilities;  /* Khả năng của storage */
    uint32_t flags;        /* Các cờ cấu hình */
    void *private_data;    /* Dữ liệu riêng của driver */
} storage_config_t;

/* Storage Operations */
typedef struct {
    /* Khởi tạo storage driver */
    int32_t (*init)(const storage_config_t *config);
    
    /* Đọc dữ liệu từ sector */
    int32_t (*read_sector)(uint32_t sector, uint8_t *buffer);
    
    /* Ghi dữ liệu vào sector */
    int32_t (*write_sector)(uint32_t sector, const uint8_t *buffer);
    
    /* Xóa sector (nếu hỗ trợ) */
    int32_t (*erase_sector)(uint32_t sector);
    
    /* TRIM sector (nếu hỗ trợ) */
    int32_t (*trim_sector)(uint32_t sector);
    
    /* Flush cache (nếu hỗ trợ) */
    int32_t (*flush_cache)(void);
    
    /* Kiểm tra trạng thái */
    int32_t (*get_status)(void);
    
    /* Đóng storage driver */
    int32_t (*deinit)(void);
} storage_ops_t;

/* Storage Driver Structure */
typedef struct {
    storage_config_t config;  /* Cấu hình */
    storage_ops_t ops;       /* Các operation */
    void *private_data;      /* Dữ liệu riêng của driver */
} storage_driver_t;

/* Storage Status */
typedef struct {
    bool is_ready;          /* Sẵn sàng sử dụng */
    bool is_write_protected; /* Bảo vệ ghi */
    uint32_t error_count;   /* Số lỗi */
    uint32_t bad_sectors;   /* Số sector lỗi */
} storage_status_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Đăng ký storage driver
 * 
 * @param driver Con trỏ đến storage driver
 * @return STORAGE_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t storage_register_driver(storage_driver_t *driver);

/**
 * @brief Hủy đăng ký storage driver
 * 
 * @param driver Con trỏ đến storage driver
 * @return STORAGE_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t storage_unregister_driver(storage_driver_t *driver);

/**
 * @brief Lấy storage driver theo loại
 * 
 * @param type Loại storage cần lấy
 * @return Con trỏ đến storage driver nếu thành công, NULL nếu thất bại
 */
storage_driver_t *storage_get_driver(uint8_t type);

/**
 * @brief Lấy danh sách các storage driver đã đăng ký
 * 
 * @param drivers Mảng lưu danh sách driver
 * @param count Con trỏ đến biến lưu số lượng driver
 * @return STORAGE_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t storage_get_drivers(storage_driver_t **drivers, uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif /* __STORAGE_INTERFACE_H */ 