/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File chứa các khai báo private của module IP Driver.
 *   Các hàm này chỉ được sử dụng trong nội bộ module.
 *********************************************************************/

#ifndef IP_DRIVER_PRIVATE_H
#define IP_DRIVER_PRIVATE_H

#include <stdint.h>
#include "../common/common_types.h"
#include "ip_driver.h"

/*********************************************************************
 * Private Types
 *********************************************************************/

/**
 * @brief IP Driver Context
 * 
 * This structure contains all the private data needed by the IP driver.
 */
typedef struct {
    ip_config_t config;  /* Driver configuration */
    bool is_ready;  /* Driver ready state */
    uint32_t last_error;  /* Last error code */
} ip_context_t;

/*********************************************************************
 * Private Constants
 *********************************************************************/
#define IP_MAX_SECTOR_SIZE 4096  /* Maximum sector size in bytes */
#define IP_MIN_SECTOR_SIZE 512   /* Minimum sector size in bytes */
#define IP_MAX_SECTORS 0xFFFFFFFF  /* Maximum number of sectors */

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Initialize IP driver context
 * 
 * @param ctx Pointer to context structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_init(ip_context_t *ctx);

/**
 * @brief Deinitialize IP driver context
 * 
 * @param ctx Pointer to context structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_deinit(ip_context_t *ctx);

/**
 * @brief Read a sector from the image file
 * 
 * @param ctx Pointer to context structure
 * @param sector_number Sector number to read
 * @param buffer Buffer to store sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_read_sector(ip_context_t *ctx, uint32_t sector_number, uint8_t *buffer);

/**
 * @brief Write a sector to the image file
 * 
 * @param ctx Pointer to context structure
 * @param sector_number Sector number to write
 * @param buffer Buffer containing sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_write_sector(ip_context_t *ctx, uint32_t sector_number, const uint8_t *buffer);

/**
 * @brief Get device information
 * 
 * @param ctx Pointer to context structure
 * @param info Pointer to device info structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_get_device_info(ip_context_t *ctx, ip_device_info_t *info);

/**
 * @brief Reset IP driver context
 * 
 * @param ctx Pointer to context structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_private_reset(ip_context_t *ctx);

#endif /* IP_DRIVER_PRIVATE_H */

/*********************************************************************
 * UUID: 4f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/ 