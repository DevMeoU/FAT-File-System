/*********************************************************************
 * Module HAL - Định nghĩa API trừu tượng hóa thao tác đọc/ghi sector
 *********************************************************************/

#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "../ip_driver/ip_driver.h"

/* HAL Constants */
#define HAL_MIN_TIMEOUT 100
#define HAL_MAX_TIMEOUT 1000

/* HAL Error Codes */
#define HAL_SUCCESS 0
#define HAL_ERROR_INVALID -1
#define HAL_ERROR_IO -2
#define HAL_ERROR_TIMEOUT -3
#define HAL_ERROR_BUSY -4
#define HAL_ERROR_NOT_FOUND -5
#define HAL_ERROR_DISK_FULL -6

/* HAL Transfer Modes */
typedef enum {
    HAL_TRANSFER_MODE_POLLING = 0,
    HAL_TRANSFER_MODE_INTERRUPT,
    HAL_TRANSFER_MODE_DMA
} hal_transfer_mode_t;

/* HAL Status */
typedef struct {
    bool is_initialized;
    bool is_busy;
    int32_t last_error;
} hal_status_t;

/* HAL Device Info */
typedef struct {
    uint32_t device_id;
    uint32_t manufacturer_id;
    uint32_t version;
    uint32_t capabilities;
} hal_device_info_t;

/* HAL Configuration */
typedef struct {
    hal_transfer_mode_t mode;
    uint32_t timeout;
    ip_config_t ip_config;    // IP Driver config
    uint32_t cache_size;      // Kích thước cache (số sector)
    uint8_t* cache_buffer;    // Buffer cache
    uint32_t* cache_map;      // Ánh xạ sector trong cache
    uint8_t* dirty_flags;     // Cờ báo sector đã thay đổi
} hal_config_t;

/* Các hàm giao diện */
int32_t hal_init(hal_config_t *config);
int32_t hal_deinit(void);
int32_t hal_read_sector(uint32_t sector_number, uint8_t* buffer);
int32_t hal_write_sector(uint32_t sector_number, const uint8_t* buffer);
int32_t hal_sync(void);
int32_t hal_read(uint8_t *buffer, uint32_t size);
int32_t hal_write(const uint8_t *buffer, uint32_t size);
int32_t hal_register_callback(void (*callback)(void *), uint32_t event_id);
int32_t hal_unregister_callback(uint32_t event_id);
int32_t hal_set_transfer_mode(hal_transfer_mode_t mode);
int32_t hal_get_device_info(hal_device_info_t *info);
int32_t hal_get_status(hal_status_t *status);
int32_t hal_reset(void);

#endif /* HAL_H */

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
