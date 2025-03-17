/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File triển khai các hàm của module IP driver.
 *   Module này cung cấp các hàm cơ bản để tương tác với thiết bị.
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
static ip_context_t ip_ctx;

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
    if (config == NULL) {
        return IP_ERROR_INVALID;
    }

    /* Initialize context */
    memset(&ip_ctx, 0, sizeof(ip_context_t));
    memcpy(&ip_ctx.config, config, sizeof(ip_config_t));

    /* Initialize IP driver */
    int32_t status = ip_driver_private_init(&ip_ctx);
    if (status != IP_SUCCESS) {
        return status;
    }

    return IP_SUCCESS;
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
    /* Deinitialize IP driver */
    int32_t status = ip_driver_private_deinit(&ip_ctx);
    if (status != IP_SUCCESS) {
        return status;
    }

    /* Clear context */
    memset(&ip_ctx, 0, sizeof(ip_context_t));

    return IP_SUCCESS;
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
int32_t ip_driver_read_sector(uint32_t sector_num, uint8_t *buffer)
{
    if (buffer == NULL) {
        return IP_ERROR_INVALID;
    }

    /* Read sector */
    int32_t status = ip_driver_private_read_sector(&ip_ctx, sector_num, buffer);
    if (status != IP_SUCCESS) {
        return status;
    }

    return IP_SUCCESS;
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
int32_t ip_driver_write_sector(uint32_t sector_num, const uint8_t *buffer)
{
    if (buffer == NULL) {
        return IP_ERROR_INVALID;
    }

    /* Write sector */
    int32_t status = ip_driver_private_write_sector(&ip_ctx, sector_num, buffer);
    if (status != IP_SUCCESS) {
        return status;
    }

    return IP_SUCCESS;
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
    if (info == NULL) {
        return IP_ERROR_INVALID;
    }

    /* Get device info */
    int32_t status = ip_driver_private_get_device_info(&ip_ctx, info);
    if (status != IP_SUCCESS) {
        return status;
    }

    return IP_SUCCESS;
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
    /* Reset IP driver */
    int32_t status = ip_driver_private_reset(&ip_ctx);
    if (status != IP_SUCCESS) {
        return status;
    }

    return IP_SUCCESS;
}

/*********************************************************************
 * Module IP Driver - Triển khai các hàm I/O cấp thấp
 *********************************************************************/

/* Khởi tạo IP Driver */
int ip_init(IpControl* ip_ctrl, const char* img_path, uint32_t sector_size) {
    if (!ip_ctrl || !img_path || sector_size == 0) {
        return IP_ERROR_IO;
    }

    // Mở file img
    ip_ctrl->fp = fopen(img_path, "rb+");
    if (!ip_ctrl->fp) {
        return IP_ERROR_IO;
    }

    // Lưu thông tin
    ip_ctrl->img_path = strdup(img_path);
    ip_ctrl->sector_size = sector_size;

    // Tính tổng số sector
    fseek(ip_ctrl->fp, 0, SEEK_END);
    long file_size = ftell(ip_ctrl->fp);
    ip_ctrl->total_sectors = file_size / sector_size;

    return IP_SUCCESS;
}

/* Đọc một sector */
int ip_read_sector(IpControl* ip_ctrl, uint32_t sector_num, void* buffer) {
    if (!ip_ctrl || !buffer || sector_num >= ip_ctrl->total_sectors) {
        return IP_ERROR_IO;
    }

    // Định vị sector
    if (fseek(ip_ctrl->fp, sector_num * ip_ctrl->sector_size, SEEK_SET) != 0) {
        return IP_ERROR_IO;
    }

    // Đọc sector
    size_t bytes_read = fread(buffer, 1, ip_ctrl->sector_size, ip_ctrl->fp);
    if (bytes_read != ip_ctrl->sector_size) {
        return IP_ERROR_IO;
    }

    return IP_SUCCESS;
}

/* Ghi một sector */
int ip_write_sector(IpControl* ip_ctrl, uint32_t sector_num, const void* buffer) {
    if (!ip_ctrl || !buffer || sector_num >= ip_ctrl->total_sectors) {
        return IP_ERROR_IO;
    }

    // Định vị sector
    if (fseek(ip_ctrl->fp, sector_num * ip_ctrl->sector_size, SEEK_SET) != 0) {
        return IP_ERROR_IO;
    }

    // Ghi sector
    size_t bytes_written = fwrite(buffer, 1, ip_ctrl->sector_size, ip_ctrl->fp);
    if (bytes_written != ip_ctrl->sector_size) {
        return IP_ERROR_IO;
    }

    // Đảm bảo dữ liệu được ghi xuống đĩa
    fflush(ip_ctrl->fp);

    return IP_SUCCESS;
}

/* Dọn dẹp */
void ip_cleanup(IpControl* ip_ctrl) {
    if (ip_ctrl) {
        if (ip_ctrl->fp) {
            fclose(ip_ctrl->fp);
        }
        if (ip_ctrl->img_path) {
            free(ip_ctrl->img_path);
        }
        memset(ip_ctrl, 0, sizeof(IpControl));
    }
}
