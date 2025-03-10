/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm public của module IP Driver.
 *   Các hàm này được sử dụng bởi các module khác.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common/common_types.h"
#include "ip_driver.h"
#include "ip_driver_private.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
static bool ip_initialized = false;  /* IP driver initialization state */
static ip_context_t ip_ctx;  /* IP driver context */

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

/**
 * @brief Initialize IP driver
 * 
 * This function initializes the IP driver by:
 * 1. Validating the configuration
 * 2. Initializing the context
 * 3. Copying the configuration
 * 4. Initializing the private context
 * 
 * @param config Pointer to configuration structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_init(ip_config_t *config)
{
    if (!config) {
        return IP_ERROR_INVALID;
    }

    /* Initialize context */
    memset(&ip_ctx, 0, sizeof(ip_context_t));
    memcpy(&ip_ctx.config, config, sizeof(ip_config_t));

    /* Initialize private context */
    int32_t status = ip_driver_private_init(&ip_ctx);
    if (status != IP_ERROR_SUCCESS) {
        return status;
    }

    ip_initialized = true;
    return IP_ERROR_SUCCESS;
}

/**
 * @brief Deinitialize IP driver
 * 
 * This function deinitializes the IP driver by:
 * 1. Checking initialization state
 * 2. Deinitializing the private context
 * 3. Resetting the initialization state
 * 
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_deinit(void)
{
    if (!ip_initialized) {
        return IP_ERROR_SUCCESS;
    }

    /* Deinitialize private context */
    int32_t status = ip_driver_private_deinit(&ip_ctx);
    if (status != IP_ERROR_SUCCESS) {
        return status;
    }

    ip_initialized = false;
    return IP_ERROR_SUCCESS;
}

/**
 * @brief Read a sector from the image file
 * 
 * This function reads a sector from the image file by:
 * 1. Validating the parameters
 * 2. Checking initialization state
 * 3. Reading the sector data
 * 
 * @param sector_number Sector number to read
 * @param buffer Buffer to store sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_read_sector(uint32_t sector_number, uint8_t *buffer)
{
    if (!buffer) {
        return IP_ERROR_INVALID;
    }

    if (!ip_initialized) {
        return IP_ERROR_NOT_READY;
    }

    return ip_driver_private_read_sector(&ip_ctx, sector_number, buffer);
}

/**
 * @brief Write a sector to the image file
 * 
 * This function writes a sector to the image file by:
 * 1. Validating the parameters
 * 2. Checking initialization state
 * 3. Writing the sector data
 * 
 * @param sector_number Sector number to write
 * @param buffer Buffer containing sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_write_sector(uint32_t sector_number, const uint8_t *buffer)
{
    if (!buffer) {
        return IP_ERROR_INVALID;
    }

    if (!ip_initialized) {
        return IP_ERROR_NOT_READY;
    }

    return ip_driver_private_write_sector(&ip_ctx, sector_number, buffer);
}

/**
 * @brief Get device information
 * 
 * This function gets the device information by:
 * 1. Validating the parameters
 * 2. Checking initialization state
 * 3. Getting the device info
 * 
 * @param info Pointer to device info structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_get_device_info(ip_device_info_t *info)
{
    if (!info) {
        return IP_ERROR_INVALID;
    }

    if (!ip_initialized) {
        return IP_ERROR_NOT_READY;
    }

    return ip_driver_private_get_device_info(&ip_ctx, info);
}

/**
 * @brief Reset IP driver
 * 
 * This function resets the IP driver by:
 * 1. Checking initialization state
 * 2. Resetting the private context
 * 
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_reset(void)
{
    if (!ip_initialized) {
        return IP_ERROR_NOT_READY;
    }

    return ip_driver_private_reset(&ip_ctx);
}

/*********************************************************************
 * UUID: 4f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/
