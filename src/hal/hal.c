/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Hardware Abstraction Layer (HAL) module cung cấp các hàm trung gian
 *   để truy cập phần cứng thông qua IP Driver.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common/common_types.h"
#include "hal.h"
#include "../ip_driver/ip_driver.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
static bool hal_initialized = false;
static hal_config_t hal_config;
static uint32_t sector_size = 0;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t hal_init(hal_config_t* config) {
    if (!config || !config->cache_size || !config->ip_config.img_path || !config->ip_config.sector_size) {
        return HAL_ERROR_INVALID;
    }

    /* Initialize IP driver */
    int32_t status = ip_driver_init(&config->ip_config);
    if (status != IP_ERROR_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Save configuration */
    memcpy(&hal_config, config, sizeof(hal_config_t));
    sector_size = config->ip_config.sector_size;

    /* Allocate cache */
    hal_config.cache_buffer = (uint8_t*)malloc(sector_size * config->cache_size);
    hal_config.cache_map = (uint32_t*)malloc(sizeof(uint32_t) * config->cache_size);
    hal_config.dirty_flags = (uint8_t*)malloc(config->cache_size);

    if (!hal_config.cache_buffer || !hal_config.cache_map || !hal_config.dirty_flags) {
        hal_deinit();
        return HAL_ERROR_IO;
    }

    /* Initialize cache */
    memset(hal_config.cache_buffer, 0, sector_size * config->cache_size);
    memset(hal_config.cache_map, 0xFF, sizeof(uint32_t) * config->cache_size);
    memset(hal_config.dirty_flags, 0, config->cache_size);

    hal_initialized = true;
    return HAL_SUCCESS;
}

int32_t hal_deinit(void) {
    if (!hal_initialized) {
        return HAL_SUCCESS;
    }

    /* Sync cache */
    hal_sync();

    /* Free cache */
    if (hal_config.cache_buffer) {
        free(hal_config.cache_buffer);
        hal_config.cache_buffer = NULL;
    }
    if (hal_config.cache_map) {
        free(hal_config.cache_map);
        hal_config.cache_map = NULL;
    }
    if (hal_config.dirty_flags) {
        free(hal_config.dirty_flags);
        hal_config.dirty_flags = NULL;
    }

    /* Deinitialize IP driver */
    ip_driver_deinit();

    hal_initialized = false;
    return HAL_SUCCESS;
}

int32_t hal_read_sector(uint32_t sector_number, uint8_t* buffer) {
    if (!buffer) {
        return HAL_ERROR_INVALID;
    }

    if (!hal_initialized) {
        return HAL_ERROR_NOT_READY;
    }

    /* Check cache */
    for (uint32_t i = 0; i < hal_config.cache_size; i++) {
        if (hal_config.cache_map[i] == sector_number) {
            /* Cache hit */
            memcpy(buffer, hal_config.cache_buffer + (i * sector_size), sector_size);
            return HAL_SUCCESS;
        }
    }

    /* Cache miss - find empty or LRU slot */
    uint32_t slot = 0;
    for (uint32_t i = 0; i < hal_config.cache_size; i++) {
        if (hal_config.cache_map[i] == 0xFFFFFFFF) {
            slot = i;
            break;
        }
    }

    /* Write back dirty sector */
    if (hal_config.dirty_flags[slot]) {
        int32_t status = ip_driver_write_sector(hal_config.cache_map[slot], 
                                              hal_config.cache_buffer + (slot * sector_size));
        if (status != IP_ERROR_SUCCESS) {
            return HAL_ERROR_IO;
        }
        hal_config.dirty_flags[slot] = 0;
    }

    /* Read new sector */
    int32_t status = ip_driver_read_sector(sector_number, 
                                         hal_config.cache_buffer + (slot * sector_size));
    if (status != IP_ERROR_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Update cache */
    hal_config.cache_map[slot] = sector_number;
    memcpy(buffer, hal_config.cache_buffer + (slot * sector_size), sector_size);

    return HAL_SUCCESS;
}

int32_t hal_write_sector(uint32_t sector_number, const uint8_t* buffer) {
    if (!buffer) {
        return HAL_ERROR_INVALID;
    }

    if (!hal_initialized) {
        return HAL_ERROR_NOT_READY;
    }

    /* Find cache slot */
    uint32_t slot = 0;
    bool found = false;

    for (uint32_t i = 0; i < hal_config.cache_size; i++) {
        if (hal_config.cache_map[i] == sector_number) {
            slot = i;
            found = true;
            break;
        }
        if (hal_config.cache_map[i] == 0xFFFFFFFF) {
            slot = i;
        }
    }

    /* Write back dirty sector if needed */
    if (!found && hal_config.dirty_flags[slot]) {
        int32_t status = ip_driver_write_sector(hal_config.cache_map[slot],
                                              hal_config.cache_buffer + (slot * sector_size));
        if (status != IP_ERROR_SUCCESS) {
            return HAL_ERROR_IO;
        }
    }

    /* Update cache */
    memcpy(hal_config.cache_buffer + (slot * sector_size), buffer, sector_size);
    hal_config.cache_map[slot] = sector_number;
    hal_config.dirty_flags[slot] = 1;

    return HAL_SUCCESS;
}

int32_t hal_sync(void) {
    if (!hal_initialized) {
        return HAL_ERROR_NOT_READY;
    }

    /* Write back all dirty sectors */
    for (uint32_t i = 0; i < hal_config.cache_size; i++) {
        if (hal_config.dirty_flags[i] && hal_config.cache_map[i] != 0xFFFFFFFF) {
            int32_t status = ip_driver_write_sector(hal_config.cache_map[i],
                                                  hal_config.cache_buffer + (i * sector_size));
            if (status != IP_ERROR_SUCCESS) {
                return HAL_ERROR_IO;
            }
            hal_config.dirty_flags[i] = 0;
        }
    }

    return HAL_SUCCESS;
}

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/

