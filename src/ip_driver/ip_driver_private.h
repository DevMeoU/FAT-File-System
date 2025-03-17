/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File header chứa các định nghĩa và khai báo private của module IP driver.
 *   Các định nghĩa và khai báo này chỉ được sử dụng nội bộ trong module.
 *********************************************************************/

#ifndef IP_DRIVER_PRIVATE_H
#define IP_DRIVER_PRIVATE_H

/*********************************************************************
 * Include Files
 *********************************************************************/
#include "../common/common_types.h"
#include "ip_driver.h"

/*********************************************************************
 * Private Definitions
 *********************************************************************/
#define IP_MAX_BUFFER_SIZE 4096

/*********************************************************************
 * Private Types
 *********************************************************************/
typedef struct {
    ip_config_t config;
    bool is_initialized;
    bool is_busy;
    uint32_t transfer_count;
    uint32_t error_count;
    uint32_t last_error;
    uint8_t buffer[IP_MAX_BUFFER_SIZE];
} ip_context_t;

/*********************************************************************
 * Private Function Declarations
 *********************************************************************/
int32_t ip_driver_private_init(ip_context_t *ctx);
int32_t ip_driver_private_deinit(ip_context_t *ctx);
int32_t ip_driver_private_read_sector(ip_context_t *ctx, uint32_t sector_num, uint8_t *buffer);
int32_t ip_driver_private_write_sector(ip_context_t *ctx, uint32_t sector_num, const uint8_t *buffer);
int32_t ip_driver_private_get_device_info(ip_context_t *ctx, ip_device_info_t *info);
int32_t ip_driver_private_reset(ip_context_t *ctx);

#endif /* IP_DRIVER_PRIVATE_H */

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/ 