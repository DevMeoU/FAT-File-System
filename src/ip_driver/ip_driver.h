/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File chứa các khai báo public của module IP Driver.
 *   Các hàm này được sử dụng bởi các module khác.
 *********************************************************************/

#ifndef IP_DRIVER_H
#define IP_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "../common/common_types.h"

/*********************************************************************
 * Error Codes
 *********************************************************************/
#define IP_ERROR_SUCCESS 0
#define IP_ERROR_INVALID -1
#define IP_ERROR_IO -2
#define IP_ERROR_NOT_FOUND -3
#define IP_ERROR_ACCESS_DENIED -4
#define IP_ERROR_ALREADY_EXISTS -5
#define IP_ERROR_NOT_EMPTY -6
#define IP_ERROR_DISK_FULL -7
#define IP_ERROR_NOT_READY -8

/*********************************************************************
 * Data Types
 *********************************************************************/

/* IP Driver Configuration */
typedef struct {
    const char *img_path;  /* Path to image file */
    uint32_t sector_size;  /* Size of each sector in bytes */
    uint32_t total_sectors;  /* Total number of sectors */
    uint32_t current_sector;  /* Current sector position */
} ip_config_t;

/* IP Driver Device Info */
typedef struct {
    uint32_t device_id;  /* Unique device identifier */
    uint32_t manufacturer_id;  /* Manufacturer identifier */
    uint32_t version;  /* Device version */
    uint32_t capabilities;  /* Device capabilities */
} ip_device_info_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Initialize IP driver with configuration
 * 
 * @param config Pointer to configuration structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_init(ip_config_t *config);

/**
 * @brief Deinitialize IP driver
 * 
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_deinit(void);

/**
 * @brief Read a sector from the image file
 * 
 * @param sector_number Sector number to read
 * @param buffer Buffer to store sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_read_sector(uint32_t sector_number, uint8_t *buffer);

/**
 * @brief Write a sector to the image file
 * 
 * @param sector_number Sector number to write
 * @param buffer Buffer containing sector data
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_write_sector(uint32_t sector_number, const uint8_t *buffer);

/**
 * @brief Get device information
 * 
 * @param info Pointer to device info structure
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_get_device_info(ip_device_info_t *info);

/**
 * @brief Reset IP driver to initial state
 * 
 * @return int32_t IP_ERROR_SUCCESS on success, error code otherwise
 */
int32_t ip_driver_reset(void);

#endif /* IP_DRIVER_H */

/*********************************************************************
 * UUID: 4f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/
