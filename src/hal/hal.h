#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include "../common/common_types.h"
#include "../ip_driver/ip_driver.h"

/* HAL Error Codes */
#define HAL_ERROR_SUCCESS 0
#define HAL_ERROR_INVALID -1
#define HAL_ERROR_IO -2
#define HAL_ERROR_NOT_FOUND -3
#define HAL_ERROR_ACCESS_DENIED -4
#define HAL_ERROR_ALREADY_EXISTS -5
#define HAL_ERROR_NOT_EMPTY -6
#define HAL_ERROR_DISK_FULL -7

/* HAL Configuration */
typedef struct {
    ip_config_t* driver;
    uint32_t sector_size;
    uint32_t cache_size;
    uint8_t* cache_buffer;
} hal_config_t;

/* Public Function Prototypes */
int32_t hal_init(hal_config_t *config);
int32_t hal_deinit(void);
int32_t hal_read_sector(uint32_t sector_number, uint8_t *buffer);
int32_t hal_write_sector(uint32_t sector_number, const uint8_t *buffer);
int32_t hal_sync(void);

#endif /* HAL_H */

/*********************************************************************
 * UUID: 2b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/
