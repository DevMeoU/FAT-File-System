/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm private của module IP Driver.
 *   Các hàm này chỉ được sử dụng trong nội bộ module.
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
 * Private Function Implementations
 *********************************************************************/

/**
 * @brief Initialize IP driver context
 * 
 * This function initializes the IP driver context by:
 * 1. Validating the sector size
 * 2. Opening the image file
 * 3. Getting the total number of sectors
 * 4. Setting up the initial state
 * 
 * @param ctx Pointer to context structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_init(ip_context_t *ctx)
{
    if (!ctx) {
        return IP_ERROR_INVALID;
    }

    /* Validate sector size */
    if (ctx->config.sector_size < IP_MIN_SECTOR_SIZE || 
        ctx->config.sector_size > IP_MAX_SECTOR_SIZE) {
        return IP_ERROR_INVALID;
    }

    /* Open image file */
    ctx->config.img_file = fopen(ctx->config.img_path, "rb+");
    if (!ctx->config.img_file) {
        return IP_ERROR_IO;
    }

    /* Get file size */
    fseek(ctx->config.img_file, 0, SEEK_END);
    ctx->config.total_sectors = ftell(ctx->config.img_file) / ctx->config.sector_size;
    fseek(ctx->config.img_file, 0, SEEK_SET);

    /* Initialize context */
    ctx->config.current_sector = 0;
    ctx->is_ready = true;
    ctx->last_error = IP_ERROR_SUCCESS;

    return IP_ERROR_SUCCESS;
}

/**
 * @brief Deinitialize IP driver context
 * 
 * This function deinitializes the IP driver context by:
 * 1. Closing the image file
 * 2. Resetting the context structure
 * 
 * @param ctx Pointer to context structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_deinit(ip_context_t *ctx)
{
    if (!ctx) {
        return IP_ERROR_INVALID;
    }

    /* Close image file */
    if (ctx->config.img_file) {
        fclose(ctx->config.img_file);
        ctx->config.img_file = NULL;
    }

    /* Reset context */
    memset(ctx, 0, sizeof(ip_context_t));
    return IP_ERROR_SUCCESS;
}

/**
 * @brief Read a sector from the image file
 * 
 * This function reads a sector from the image file by:
 * 1. Validating the parameters
 * 2. Checking the sector number
 * 3. Reading the sector data
 * 4. Updating the current position
 * 
 * @param ctx Pointer to context structure
 * @param sector_number Sector number to read
 * @param buffer Buffer to store sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_read_sector(ip_context_t *ctx, uint32_t sector_number, uint8_t *buffer)
{
    if (!ctx || !buffer) {
        return IP_ERROR_INVALID;
    }

    if (!ctx->is_ready) {
        return IP_ERROR_NOT_READY;
    }

    /* Check sector number */
    if (sector_number >= ctx->config.total_sectors) {
        return IP_ERROR_INVALID;
    }

    /* Read sector */
    fseek(ctx->config.img_file, sector_number * ctx->config.sector_size, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, ctx->config.sector_size, ctx->config.img_file);
    if (bytes_read != ctx->config.sector_size) {
        ctx->last_error = IP_ERROR_IO;
        return IP_ERROR_IO;
    }

    ctx->config.current_sector = sector_number;
    ctx->last_error = IP_ERROR_SUCCESS;
    return IP_ERROR_SUCCESS;
}

/**
 * @brief Write a sector to the image file
 * 
 * This function writes a sector to the image file by:
 * 1. Validating the parameters
 * 2. Checking the sector number
 * 3. Writing the sector data
 * 4. Updating the current position
 * 
 * @param ctx Pointer to context structure
 * @param sector_number Sector number to write
 * @param buffer Buffer containing sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_write_sector(ip_context_t *ctx, uint32_t sector_number, const uint8_t *buffer)
{
    if (!ctx || !buffer) {
        return IP_ERROR_INVALID;
    }

    if (!ctx->is_ready) {
        return IP_ERROR_NOT_READY;
    }

    /* Check sector number */
    if (sector_number >= ctx->config.total_sectors) {
        return IP_ERROR_INVALID;
    }

    /* Write sector */
    fseek(ctx->config.img_file, sector_number * ctx->config.sector_size, SEEK_SET);
    size_t bytes_written = fwrite(buffer, 1, ctx->config.sector_size, ctx->config.img_file);
    if (bytes_written != ctx->config.sector_size) {
        ctx->last_error = IP_ERROR_IO;
        return IP_ERROR_IO;
    }

    ctx->config.current_sector = sector_number;
    ctx->last_error = IP_ERROR_SUCCESS;
    return IP_ERROR_SUCCESS;
}

/**
 * @brief Get device information
 * 
 * This function gets the device information by:
 * 1. Validating the parameters
 * 2. Filling in the device info structure
 * 
 * @param ctx Pointer to context structure
 * @param info Pointer to device info structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_get_device_info(ip_context_t *ctx, ip_device_info_t *info)
{
    if (!ctx || !info) {
        return IP_ERROR_INVALID;
    }

    if (!ctx->is_ready) {
        return IP_ERROR_NOT_READY;
    }

    /* Get device info */
    info->device_id = 0;
    info->manufacturer_id = 0;
    info->version = 0;
    info->capabilities = 0;

    ctx->last_error = IP_ERROR_SUCCESS;
    return IP_ERROR_SUCCESS;
}

/**
 * @brief Reset IP driver context
 * 
 * This function resets the IP driver context by:
 * 1. Validating the parameters
 * 2. Resetting the file position
 * 3. Updating the current sector
 * 
 * @param ctx Pointer to context structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_reset(ip_context_t *ctx)
{
    if (!ctx) {
        return IP_ERROR_INVALID;
    }

    if (!ctx->is_ready) {
        return IP_ERROR_NOT_READY;
    }

    /* Reset file position */
    fseek(ctx->config.img_file, 0, SEEK_SET);
    ctx->config.current_sector = 0;

    ctx->last_error = IP_ERROR_SUCCESS;
    return IP_ERROR_SUCCESS;
}

/*********************************************************************
 * UUID: 4f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/ 