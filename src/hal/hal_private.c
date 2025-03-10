/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm private của module HAL.
 *   Các hàm này chỉ được sử dụng nội bộ trong module.
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
 * Private Function Implementations
 *********************************************************************/

int32_t hal_private_init(hal_context_t *ctx)
{
    if (!ctx) {
        return HAL_ERROR_INVALID;
    }

    /* Initialize IP driver */
    int32_t status = ip_driver_init(ctx->config.driver);
    if (status != IP_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Initialize status */
    ctx->status.is_initialized = true;
    ctx->status.is_busy = false;
    ctx->status.transfer_count = 0;
    ctx->status.error_count = 0;
    ctx->status.last_error = 0;

    /* Initialize transfer mode */
    ctx->transfer_mode = HAL_MODE_POLLING;

    /* Initialize callbacks */
    memset(ctx->callbacks, 0, sizeof(ctx->callbacks));
    memset(ctx->callback_params, 0, sizeof(ctx->callback_params));

    return HAL_ERROR_SUCCESS;
}

int32_t hal_private_deinit(hal_context_t *ctx)
{
    if (!ctx) {
        return HAL_ERROR_INVALID;
    }

    /* Deinitialize IP driver */
    int32_t status = ip_driver_deinit();
    if (status != IP_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Reset context */
    memset(ctx, 0, sizeof(hal_context_t));
    return HAL_ERROR_SUCCESS;
}

int32_t hal_private_read(hal_context_t *ctx, uint8_t *buffer, uint32_t size, uint32_t timeout)
{
    if (!ctx || !buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE || 
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return HAL_ERROR_INVALID;
    }

    if (!ctx->status.is_initialized) {
        return HAL_ERROR_NOT_READY;
    }

    if (ctx->status.is_busy) {
        return HAL_ERROR_BUSY;
    }

    ctx->status.is_busy = true;
    ctx->status.transfer_count++;

    /* Read based on transfer mode */
    int32_t status;
    switch (ctx->transfer_mode) {
        case HAL_MODE_POLLING:
            status = ip_driver_read(ctx->config.driver, buffer, size);
            break;
        case HAL_MODE_INTERRUPT:
            status = ip_driver_read_interrupt(ctx->config.driver, buffer, size);
            break;
        case HAL_MODE_DMA:
            status = ip_driver_read_dma(ctx->config.driver, buffer, size);
            break;
        default:
            status = HAL_ERROR_INVALID;
            break;
    }

    if (status != IP_SUCCESS) {
        ctx->status.error_count++;
        ctx->status.last_error = status;
        status = HAL_ERROR_IO;
    }

    ctx->status.is_busy = false;
    return status;
}

int32_t hal_private_write(hal_context_t *ctx, const uint8_t *buffer, uint32_t size, uint32_t timeout)
{
    if (!ctx || !buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE ||
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return HAL_ERROR_INVALID;
    }

    if (!ctx->status.is_initialized) {
        return HAL_ERROR_NOT_READY;
    }

    if (ctx->status.is_busy) {
        return HAL_ERROR_BUSY;
    }

    ctx->status.is_busy = true;
    ctx->status.transfer_count++;

    /* Write based on transfer mode */
    int32_t status;
    switch (ctx->transfer_mode) {
        case HAL_MODE_POLLING:
            status = ip_driver_write(ctx->config.driver, buffer, size);
            break;
        case HAL_MODE_INTERRUPT:
            status = ip_driver_write_interrupt(ctx->config.driver, buffer, size);
            break;
        case HAL_MODE_DMA:
            status = ip_driver_write_dma(ctx->config.driver, buffer, size);
            break;
        default:
            status = HAL_ERROR_INVALID;
            break;
    }

    if (status != IP_SUCCESS) {
        ctx->status.error_count++;
        ctx->status.last_error = status;
        status = HAL_ERROR_IO;
    }

    ctx->status.is_busy = false;
    return status;
}

int32_t hal_private_register_callback(hal_context_t *ctx, void (*callback)(void *), uint32_t event_id)
{
    if (!ctx || !callback) {
        return HAL_ERROR_INVALID;
    }

    /* Find empty slot */
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (ctx->callbacks[i] == NULL) {
            ctx->callbacks[i] = callback;
            ctx->callback_params[i] = NULL;
            return HAL_ERROR_SUCCESS;
        }
    }

    return HAL_ERROR_DISK_FULL;
}

int32_t hal_private_unregister_callback(hal_context_t *ctx, uint32_t event_id)
{
    if (!ctx) {
        return HAL_ERROR_INVALID;
    }

    /* Find callback */
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (ctx->callbacks[i] != NULL) {
            ctx->callbacks[i] = NULL;
            ctx->callback_params[i] = NULL;
            return HAL_ERROR_SUCCESS;
        }
    }

    return HAL_ERROR_NOT_FOUND;
}

int32_t hal_private_set_transfer_mode(hal_context_t *ctx, hal_transfer_mode_t mode)
{
    if (!ctx) {
        return HAL_ERROR_INVALID;
    }

    if (mode > HAL_MODE_DMA) {
        return HAL_ERROR_INVALID;
    }

    ctx->transfer_mode = mode;
    return HAL_ERROR_SUCCESS;
}

int32_t hal_private_get_device_info(hal_context_t *ctx, hal_device_info_t *info)
{
    if (!ctx || !info) {
        return HAL_ERROR_INVALID;
    }

    if (!ctx->status.is_initialized) {
        return HAL_ERROR_NOT_READY;
    }

    /* Get device info from IP driver */
    ip_device_info_t ip_info;
    int32_t status = ip_driver_get_device_info(ctx->config.driver, &ip_info);
    if (status != IP_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Copy device info */
    info->device_id = ip_info.device_id;
    info->manufacturer_id = ip_info.manufacturer_id;
    info->version = ip_info.version;
    info->capabilities = ip_info.capabilities;

    return HAL_ERROR_SUCCESS;
}

int32_t hal_private_get_status(hal_context_t *ctx, hal_status_t *status)
{
    if (!ctx || !status) {
        return HAL_ERROR_INVALID;
    }

    memcpy(status, &ctx->status, sizeof(hal_status_t));
    return HAL_ERROR_SUCCESS;
}

int32_t hal_private_reset(hal_context_t *ctx)
{
    if (!ctx) {
        return HAL_ERROR_INVALID;
    }

    if (!ctx->status.is_initialized) {
        return HAL_ERROR_NOT_READY;
    }

    /* Reset IP driver */
    int32_t status = ip_driver_reset(ctx->config.driver);
    if (status != IP_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Reset status */
    memset(&ctx->status, 0, sizeof(hal_status_t));
    ctx->status.is_initialized = true;

    /* Reset transfer mode */
    ctx->transfer_mode = HAL_MODE_POLLING;

    return HAL_ERROR_SUCCESS;
}

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/ 