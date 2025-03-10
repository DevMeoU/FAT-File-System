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
#include "hal_private.h"
#include "../ip_driver/ip_driver.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
static bool hal_initialized = false;
static hal_context_t hal_ctx;
static hal_status_t hal_status;
static hal_transfer_mode_t transfer_mode = HAL_MODE_POLLING;

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t hal_init(hal_config_t *config)
{
    if (!config || !config->driver) {
        return HAL_ERROR_INVALID;
    }

    /* Copy configuration */
    memcpy(&hal_ctx.config, config, sizeof(hal_config_t));

    /* Initialize cache */
    hal_ctx.cache_buffer = (uint8_t *)malloc(config->sector_size * config->cache_size);
    if (!hal_ctx.cache_buffer) {
        return HAL_ERROR_IO;
    }

    /* Initialize IP driver */
    int32_t status = ip_driver_init(config->driver);
    if (status != IP_SUCCESS) {
        free(hal_ctx.cache_buffer);
        hal_ctx.cache_buffer = NULL;
        return HAL_ERROR_IO;
    }

    hal_initialized = true;
    return HAL_ERROR_SUCCESS;
}

int32_t hal_deinit(void)
{
    if (!hal_initialized) {
        return HAL_ERROR_SUCCESS;
    }

    /* Close IP driver */
    int32_t status = ip_driver_deinit();
    if (status != IP_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Free cache */
    if (hal_ctx.cache_buffer) {
        free(hal_ctx.cache_buffer);
        hal_ctx.cache_buffer = NULL;
    }

    /* Reset context */
    memset(&hal_ctx, 0, sizeof(hal_context_t));
    memset(&hal_status, 0, sizeof(hal_status_t));
    hal_initialized = false;

    return HAL_ERROR_SUCCESS;
}

int32_t hal_read_sector(uint32_t sector_number, uint8_t *buffer)
{
    if (!buffer) {
        return HAL_ERROR_INVALID;
    }

    /* Check cache */
    uint32_t cache_index = sector_number % hal_ctx.config.cache_size;
    uint8_t *cache_entry = hal_ctx.cache_buffer + (cache_index * hal_ctx.config.sector_size);

    /* Read from IP driver */
    int32_t status = ip_driver_read_sector(hal_ctx.config.driver, sector_number, cache_entry);
    if (status != IP_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Copy to buffer */
    memcpy(buffer, cache_entry, hal_ctx.config.sector_size);
    return HAL_ERROR_SUCCESS;
}

int32_t hal_write_sector(uint32_t sector_number, const uint8_t *buffer)
{
    if (!buffer) {
        return HAL_ERROR_INVALID;
    }

    /* Update cache */
    uint32_t cache_index = sector_number % hal_ctx.config.cache_size;
    uint8_t *cache_entry = hal_ctx.cache_buffer + (cache_index * hal_ctx.config.sector_size);
    memcpy(cache_entry, buffer, hal_ctx.config.sector_size);

    /* Write to IP driver */
    int32_t status = ip_driver_write_sector(hal_ctx.config.driver, sector_number, buffer);
    if (status != IP_SUCCESS) {
        return HAL_ERROR_IO;
    }

    return HAL_ERROR_SUCCESS;
}

int32_t hal_sync(void)
{
    if (!hal_initialized) {
        return HAL_ERROR_INVALID;
    }

    /* Flush cache */
    for (uint32_t i = 0; i < hal_ctx.config.cache_size; i++) {
        uint8_t *cache_entry = hal_ctx.cache_buffer + (i * hal_ctx.config.sector_size);
        int32_t status = ip_driver_write_sector(hal_ctx.config.driver, i, cache_entry);
        if (status != IP_SUCCESS) {
            return HAL_ERROR_IO;
        }
    }

    return HAL_ERROR_SUCCESS;
}

int32_t hal_read(void *buffer, uint32_t size, uint32_t timeout)
{
    if (!buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE || 
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    if (hal_status.is_busy) {
        return STATUS_BUSY;
    }

    hal_status.is_busy = true;
    hal_status.transfer_count++;

    // TODO: Implement actual read based on transfer mode

    hal_status.is_busy = false;
    return STATUS_SUCCESS;
}

int32_t hal_write(const void *buffer, uint32_t size, uint32_t timeout)
{
    if (!buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE ||
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    if (hal_status.is_busy) {
        return STATUS_BUSY;
    }

    hal_status.is_busy = true;
    hal_status.transfer_count++;

    // TODO: Implement actual write based on transfer mode

    hal_status.is_busy = false;
    return STATUS_SUCCESS;
}

int32_t hal_register_callback(void (*callback)(void *), uint32_t event_id)
{
    (void)event_id; // Unused parameter
    if (!callback) {
        return STATUS_INVALID_PARAMETER;
    }

    // Tìm slot trống
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (hal_ctx.callbacks[i].callback == NULL) {
            hal_ctx.callbacks[i].callback = callback;
            hal_ctx.callbacks[i].param = NULL;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ERROR;
}

int32_t hal_unregister_callback(uint32_t event_id)
{
    (void)event_id; // Unused parameter
    // Tìm callback cần xóa
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (hal_ctx.callbacks[i].callback != NULL) {
            hal_ctx.callbacks[i].callback = NULL;
            hal_ctx.callbacks[i].param = NULL;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ERROR;
}

int32_t hal_set_transfer_mode(hal_transfer_mode_t mode)
{
    if (mode > HAL_MODE_DMA) {
        return STATUS_INVALID_PARAMETER;
    }

    transfer_mode = mode;
    return STATUS_SUCCESS;
}

int32_t hal_get_device_info(hal_device_info_t *info)
{
    if (!info) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    // TODO: Get actual device info
    info->device_id = 0;
    info->manufacturer_id = 0;
    info->version = 0;
    info->capabilities = 0;

    return STATUS_SUCCESS;
}

int32_t hal_get_status(hal_status_t *status)
{
    if (!status) {
        return STATUS_INVALID_PARAMETER;
    }

    memcpy(status, &hal_status, sizeof(hal_status_t));
    return STATUS_SUCCESS;
}

int32_t hal_reset(void)
{
    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    // Reset status
    memset(&hal_status, 0, sizeof(hal_status_t));
    hal_status.is_initialized = true;

    // Reset transfer mode
    transfer_mode = HAL_MODE_POLLING;

    return STATUS_SUCCESS;
}

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/

