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
#include "../ip_driver/ip_driver.h"
#include "hal.h"
#include "hal_private.h"

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

int32_t hal_private_init(hal_context_t *ctx)
{
    if (ctx == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Initialize IP driver */
    ip_config_t ip_config;
    memset(&ip_config, 0, sizeof(ip_config));
    int32_t status = ip_driver_init(&ip_config);
    if (status != HAL_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Initialize context */
    memset(ctx, 0, sizeof(hal_context_t));
    ctx->status.is_initialized = true;
    ctx->status.is_busy = false;
    ctx->status.last_error = HAL_SUCCESS;

    return HAL_SUCCESS;
}

int32_t hal_private_deinit(hal_context_t *ctx)
{
    if (ctx == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Deinitialize IP driver */
    int32_t status = ip_driver_deinit();
    if (status != HAL_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Clear context */
    memset(ctx, 0, sizeof(hal_context_t));

    return HAL_SUCCESS;
}

int32_t hal_private_read(hal_context_t *ctx, uint8_t *buffer, uint32_t size)
{
    if (ctx == NULL || buffer == NULL || size == 0) {
        return HAL_ERROR_INVALID;
    }

    /* Check if busy */
    if (ctx->status.is_busy) {
        return HAL_ERROR_BUSY;
    }

    /* Set busy flag */
    ctx->status.is_busy = true;

    /* Read data */
    int32_t status;
    switch (ctx->config.mode) {
        case HAL_TRANSFER_MODE_POLLING:
            status = ip_driver_read_sector(0, buffer);
            break;
        case HAL_TRANSFER_MODE_INTERRUPT:
            status = ip_driver_read_sector(0, buffer);
            break;
        case HAL_TRANSFER_MODE_DMA:
            status = ip_driver_read_sector(0, buffer);
            break;
        default:
            status = HAL_ERROR_INVALID;
            break;
    }

    /* Clear busy flag */
    ctx->status.is_busy = false;

    /* Update last error */
    ctx->status.last_error = status;

    return status;
}

int32_t hal_private_write(hal_context_t *ctx, const uint8_t *buffer, uint32_t size)
{
    if (ctx == NULL || buffer == NULL || size == 0) {
        return HAL_ERROR_INVALID;
    }

    /* Check if busy */
    if (ctx->status.is_busy) {
        return HAL_ERROR_BUSY;
    }

    /* Set busy flag */
    ctx->status.is_busy = true;

    /* Write data */
    int32_t status;
    switch (ctx->config.mode) {
        case HAL_TRANSFER_MODE_POLLING:
            status = ip_driver_write_sector(0, buffer);
            break;
        case HAL_TRANSFER_MODE_INTERRUPT:
            status = ip_driver_write_sector(0, buffer);
            break;
        case HAL_TRANSFER_MODE_DMA:
            status = ip_driver_write_sector(0, buffer);
            break;
        default:
            status = HAL_ERROR_INVALID;
            break;
    }

    /* Clear busy flag */
    ctx->status.is_busy = false;

    /* Update last error */
    ctx->status.last_error = status;

    return status;
}

int32_t hal_private_register_callback(hal_context_t *ctx, void (*callback)(void *), uint32_t event_id)
{
    if (ctx == NULL || callback == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Find empty slot */
    for (int i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (ctx->callbacks[i] == NULL) {
            ctx->callbacks[i] = callback;
            return HAL_SUCCESS;
        }
    }

    return HAL_ERROR_DISK_FULL;
}

int32_t hal_private_unregister_callback(hal_context_t *ctx, uint32_t event_id)
{
    if (ctx == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Find callback */
    for (int i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (ctx->callbacks[i] != NULL) {
            ctx->callbacks[i] = NULL;
            return HAL_SUCCESS;
        }
    }

    return HAL_ERROR_NOT_FOUND;
}

int32_t hal_private_set_transfer_mode(hal_context_t *ctx, hal_transfer_mode_t mode)
{
    if (ctx == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Set transfer mode */
    ctx->config.mode = mode;

    return HAL_SUCCESS;
}

int32_t hal_private_get_device_info(hal_context_t *ctx, hal_device_info_t *info)
{
    if (ctx == NULL || info == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Get device info from IP driver */
    ip_device_info_t ip_info;
    int32_t status = ip_driver_get_device_info(&ip_info);
    if (status != HAL_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Convert IP device info to HAL device info */
    info->device_id = ip_info.device_id;
    info->manufacturer_id = ip_info.manufacturer_id;
    info->version = ip_info.version;
    info->capabilities = ip_info.capabilities;

    return HAL_SUCCESS;
}

int32_t hal_private_get_status(hal_context_t *ctx, hal_status_t *status)
{
    if (ctx == NULL || status == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Copy status */
    memcpy(status, &ctx->status, sizeof(hal_status_t));

    return HAL_SUCCESS;
}

int32_t hal_private_reset(hal_context_t *ctx)
{
    if (ctx == NULL) {
        return HAL_ERROR_INVALID;
    }

    /* Reset IP driver */
    int32_t status = ip_driver_reset();
    if (status != HAL_SUCCESS) {
        return HAL_ERROR_IO;
    }

    /* Reset context */
    memset(ctx, 0, sizeof(hal_context_t));
    ctx->status.is_initialized = true;
    ctx->status.is_busy = false;
    ctx->status.last_error = HAL_SUCCESS;

    return HAL_SUCCESS;
}

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/ 