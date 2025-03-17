/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File header chứa các định nghĩa và khai báo private của module HAL.
 *   Các định nghĩa và khai báo này chỉ được sử dụng nội bộ trong module.
 *********************************************************************/

#ifndef HAL_PRIVATE_H
#define HAL_PRIVATE_H

/*********************************************************************
 * Include Files
 *********************************************************************/
#include "../common/common_types.h"
#include "../ip_driver/ip_driver.h"
#include "hal.h"

/*********************************************************************
 * Private Definitions
 *********************************************************************/
#define HAL_MAX_CALLBACKS 16

/*********************************************************************
 * Private Types
 *********************************************************************/
typedef struct {
    hal_config_t config;
    hal_status_t status;
    void (*callbacks[HAL_MAX_CALLBACKS])(void *);
} hal_context_t;

/*********************************************************************
 * Private Function Declarations
 *********************************************************************/
int32_t hal_private_init(hal_context_t *ctx);
int32_t hal_private_deinit(hal_context_t *ctx);
int32_t hal_private_read(hal_context_t *ctx, uint8_t *buffer, uint32_t size);
int32_t hal_private_write(hal_context_t *ctx, const uint8_t *buffer, uint32_t size);
int32_t hal_private_register_callback(hal_context_t *ctx, void (*callback)(void *), uint32_t event_id);
int32_t hal_private_unregister_callback(hal_context_t *ctx, uint32_t event_id);
int32_t hal_private_set_transfer_mode(hal_context_t *ctx, hal_transfer_mode_t mode);
int32_t hal_private_get_device_info(hal_context_t *ctx, hal_device_info_t *info);
int32_t hal_private_get_status(hal_context_t *ctx, hal_status_t *status);
int32_t hal_private_reset(hal_context_t *ctx);

#endif /* HAL_PRIVATE_H */

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/ 