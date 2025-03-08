/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module HAL (Hardware Abstraction Layer) cung cấp interface để tương tác
 *   với phần cứng, cho phép truy cập các tài nguyên phần cứng một cách
 *   độc lập với nền tảng cụ thể.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hal.h"
#include "hal_private.h"
#include "../ip_driver/ip_driver.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/

/* HAL context */
static hal_context_t hal_ctx;
static hal_config_t hal_config;
static hal_status_t hal_status;
static hal_transfer_mode_t transfer_mode = HAL_MODE_POLLING;

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

/**
 * @brief Initialize ring buffer
 * 
 * @param buffer Ring buffer structure
 * @param size Buffer size
 * @return HAL_SUCCESS if successful, error code otherwise
 */
__attribute__((unused))
static int32_t hal_init_ring_buffer(hal_ring_buffer_t *buffer, uint32_t size)
{
    if (buffer == NULL || size == 0) {
        return HAL_INVALID_PARAM;
    }

    buffer->buffer = (uint8_t *)malloc(size);
    if (buffer->buffer == NULL) {
        return HAL_ERROR;
    }

    buffer->size = size;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;

    return HAL_SUCCESS;
}

/**
 * @brief Write data to ring buffer
 * 
 * @param buffer Ring buffer structure
 * @param data Data to write
 * @param size Data size
 * @return Number of bytes written
 */
static uint32_t hal_write_ring_buffer(hal_ring_buffer_t *buffer, const uint8_t *data, uint32_t size)
{
    if (buffer == NULL || data == NULL || size == 0) {
        return 0;
    }

    uint32_t written = 0;
    while (written < size && buffer->count < buffer->size) {
        buffer->buffer[buffer->tail] = data[written];
        buffer->tail = (buffer->tail + 1) % buffer->size;
        buffer->count++;
        written++;
    }

    return written;
}

/**
 * @brief Read data from ring buffer
 * 
 * @param buffer Ring buffer structure
 * @param data Buffer to store data
 * @param size Maximum data size to read
 * @return Number of bytes read
 */
static uint32_t hal_read_ring_buffer(hal_ring_buffer_t *buffer, uint8_t *data, uint32_t size)
{
    if (buffer == NULL || data == NULL || size == 0) {
        return 0;
    }

    uint32_t read = 0;
    while (read < size && buffer->count > 0) {
        data[read] = buffer->buffer[buffer->head];
        buffer->head = (buffer->head + 1) % buffer->size;
        buffer->count--;
        read++;
    }

    return read;
}

/**
 * @brief IRQ handler
 */
__attribute__((unused))
static void hal_irq_handler(void)
{
    uint32_t int_status = hal_ctx.reg->int_status;
    
    if (int_status & HAL_INT_RX_READY) {
        uint8_t data = (uint8_t)hal_ctx.reg->data;
        hal_write_ring_buffer(&hal_ctx.rx_buffer, &data, 1);
        
        // Notify registered callbacks
        for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
            if (hal_ctx.callbacks[i].callback != NULL) {
                hal_ctx.callbacks[i].callback(hal_ctx.callbacks[i].param);
            }
        }
    }
    
    if (int_status & HAL_INT_ERROR) {
        hal_ctx.error_count++;
    }
    
    // Clear interrupt status
    hal_ctx.reg->int_status = int_status;
}

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t hal_init(const hal_config_t *config)
{
    if (!config) {
        return HAL_INVALID_PARAM;
    }

    // Lưu cấu hình
    memcpy(&hal_config, config, sizeof(hal_config_t));

    // Khởi tạo status
    memset(&hal_status, 0, sizeof(hal_status_t));
    hal_status.is_initialized = true;

    // Khởi tạo IP driver nếu đang sử dụng storage IP
    if (config->storage_type == HAL_STORAGE_IP) {
        ip_config_t ip_config = {0};
        // TODO: Set up IP config
        int32_t ret = ip_driver_init(&ip_config);
        if (ret != IP_SUCCESS) {
            return HAL_ERROR;
        }
    }

    /* Initialize context */
    memset(&hal_ctx, 0, sizeof(hal_ctx));
    hal_ctx.config = *config;
    hal_ctx.reg = (hal_reg_map_t *)HAL_REG_BASE_ADDR;
    hal_ctx.state = HAL_STATE_INITIALIZED;

    /* Initialize ring buffers */
    int32_t ret = hal_init_ring_buffer(&hal_ctx.rx_buffer, HAL_RX_BUFFER_SIZE);
    if (ret != HAL_SUCCESS) {
        return ret;
    }

    ret = hal_init_ring_buffer(&hal_ctx.tx_buffer, HAL_TX_BUFFER_SIZE);
    if (ret != HAL_SUCCESS) {
        free(hal_ctx.rx_buffer.buffer);
        return ret;
    }

    /* Configure hardware */
    hal_ctx.reg->control = HAL_CONTROL_RESET;
    hal_ctx.reg->int_enable = config->interrupt_enable ? 
                             (HAL_INT_RX_READY | HAL_INT_ERROR) : 0;

    return HAL_SUCCESS;
}

int32_t hal_read_sector(uint32_t sector, uint8_t *buffer) {
    if (!buffer || !hal_status.is_initialized) {
        return HAL_INVALID_PARAM;
    }

    // Gọi hàm tương ứng theo loại storage
    switch (hal_config.storage_type) {
        case HAL_STORAGE_IP:
            return ip_read_sector(sector, buffer);
            
        case HAL_STORAGE_FLASH:
            // TODO: Implement Flash read
            return HAL_ERROR;
            
        case HAL_STORAGE_SD:
            // TODO: Implement SD read
            return HAL_ERROR;
            
        default:
            return HAL_ERROR;
    }
}

int32_t hal_write_sector(uint32_t sector, const uint8_t *buffer) {
    if (!buffer || !hal_status.is_initialized) {
        return HAL_INVALID_PARAM;
    }

    // Gọi hàm tương ứng theo loại storage
    switch (hal_config.storage_type) {
        case HAL_STORAGE_IP:
            return ip_write_sector(sector, buffer);
            
        case HAL_STORAGE_FLASH:
            // TODO: Implement Flash write
            return HAL_ERROR;
            
        case HAL_STORAGE_SD:
            // TODO: Implement SD write
            return HAL_ERROR;
            
        default:
            return HAL_ERROR;
    }
}

int32_t hal_read(void *buffer, uint32_t size, uint32_t timeout) {
    if (!buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE || 
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return HAL_INVALID_PARAM;
    }

    if (!hal_status.is_initialized) {
        return HAL_NOT_READY;
    }

    if (hal_status.is_busy) {
        return HAL_BUSY;
    }

    hal_status.is_busy = true;
    hal_status.transfer_count++;

    // TODO: Implement actual read based on transfer mode

    hal_status.is_busy = false;
    return HAL_SUCCESS;
}

int32_t hal_write(const void *buffer, uint32_t size, uint32_t timeout) {
    if (!buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE ||
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return HAL_INVALID_PARAM;
    }

    if (!hal_status.is_initialized) {
        return HAL_NOT_READY;
    }

    if (hal_status.is_busy) {
        return HAL_BUSY;
    }

    hal_status.is_busy = true;
    hal_status.transfer_count++;

    // TODO: Implement actual write based on transfer mode

    hal_status.is_busy = false;
    return HAL_SUCCESS;
}

int32_t hal_register_callback(void (*callback)(void *), uint32_t event_id)
{
    if (callback == NULL || event_id >= HAL_MAX_CALLBACKS) {
        return HAL_INVALID_PARAM;
    }

    if (hal_ctx.state != HAL_STATE_INITIALIZED) {
        return HAL_ERROR;
    }

    /* Find empty slot */
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (hal_ctx.callbacks[i].callback == NULL) {
            hal_ctx.callbacks[i].callback = callback;
            hal_ctx.callbacks[i].event_id = event_id;
            return HAL_SUCCESS;
        }
    }

    return HAL_ERROR;  /* No empty slots */
}

int32_t hal_unregister_callback(uint32_t event_id)
{
    if (event_id >= HAL_MAX_CALLBACKS) {
        return HAL_INVALID_PARAM;
    }

    if (hal_ctx.state != HAL_STATE_INITIALIZED) {
        return HAL_ERROR;
    }

    /* Find and remove callback */
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (hal_ctx.callbacks[i].event_id == event_id) {
            hal_ctx.callbacks[i].callback = NULL;
            hal_ctx.callbacks[i].event_id = 0;
            return HAL_SUCCESS;
        }
    }

    return HAL_ERROR;  /* Callback not found */
}

int32_t hal_set_transfer_mode(hal_transfer_mode_t mode) {
    if (mode > HAL_MODE_DMA) {
        return HAL_INVALID_PARAM;
    }

    transfer_mode = mode;
    return HAL_SUCCESS;
}

int32_t hal_get_device_info(hal_device_info_t *info) {
    if (!info) {
        return HAL_INVALID_PARAM;
    }

    if (!hal_status.is_initialized) {
        return HAL_NOT_READY;
    }

    // TODO: Get actual device info
    info->device_id = 0;
    info->manufacturer_id = 0;
    info->version = 0;
    info->capabilities = 0;

    return HAL_SUCCESS;
}

int32_t hal_get_status(hal_status_t *status) {
    if (!status) {
        return HAL_INVALID_PARAM;
    }

    memcpy(status, &hal_status, sizeof(hal_status_t));
    return HAL_SUCCESS;
}

int32_t hal_reset(void) {
    if (!hal_status.is_initialized) {
        return HAL_NOT_READY;
    }

    // Reset status
    memset(&hal_status, 0, sizeof(hal_status_t));
    hal_status.is_initialized = true;

    // Reset transfer mode
    transfer_mode = HAL_MODE_POLLING;

    return HAL_SUCCESS;
}

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/

