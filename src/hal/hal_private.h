#ifndef HAL_PRIVATE_H
#define HAL_PRIVATE_H

#include <stdint.h>
#include "../common/common_types.h"
#include "hal.h"

/* HAL Transfer Modes */
typedef enum {
    HAL_MODE_POLLING = 0,
    HAL_MODE_INTERRUPT = 1,
    HAL_MODE_DMA = 2
} hal_transfer_mode_t;

/* HAL Status */
typedef struct {
    bool is_initialized;
    bool is_busy;
    uint32_t transfer_count;
    uint32_t error_count;
    uint32_t last_error;
} hal_status_t;

/* HAL Context */
typedef struct {
    hal_config_t config;
    hal_status_t status;
    hal_transfer_mode_t transfer_mode;
    void (*callbacks[HAL_MAX_CALLBACKS])(void *);
    void *callback_params[HAL_MAX_CALLBACKS];
} hal_context_t;

/* HAL Device Info */
typedef struct {
    uint32_t device_id;
    uint32_t manufacturer_id;
    uint32_t version;
    uint32_t capabilities;
} hal_device_info_t;

/* HAL Constants */
#define HAL_MAX_BUFFER_SIZE 4096
#define HAL_MIN_TIMEOUT 100
#define HAL_MAX_TIMEOUT 1000
#define HAL_MAX_CALLBACKS 8

/* Private Function Prototypes */
int32_t hal_private_init(hal_context_t *ctx);
int32_t hal_private_deinit(hal_context_t *ctx);
int32_t hal_private_read(hal_context_t *ctx, uint8_t *buffer, uint32_t size, uint32_t timeout);
int32_t hal_private_write(hal_context_t *ctx, const uint8_t *buffer, uint32_t size, uint32_t timeout);
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