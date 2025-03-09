/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Hardware Abstraction Layer (HAL) module cung cấp các hàm trung gian
 *   để truy cập phần cứng thông qua IP Driver.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../common/common_types.h"
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
static bool hal_initialized = false;

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
        return STATUS_INVALID_PARAMETER;
    }

    buffer->buffer = (uint8_t *)malloc(size);
    if (buffer->buffer == NULL) {
        return STATUS_ERROR;
    }

    buffer->size = size;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;

    return STATUS_SUCCESS;
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
__attribute__((unused))
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

/**
 * @brief Quản lý cache
 * 
 * @param sector Sector number
 * @param data Data buffer
 * @param write true if write operation
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t hal_cache_manage(uint32_t sector,
                              uint8_t *data,
                              bool write)
{
    if (!data) {
        return STATUS_INVALID_PARAMETER;
    }

    // Kiểm tra cache hit
    if (hal_ctx.cache.valid && hal_ctx.cache.sector == sector) {
        if (write) {
            // Cập nhật cache và đánh dấu dirty
            memcpy(hal_ctx.cache.data, data, STORAGE_MAX_SECTOR_SIZE);
            hal_ctx.cache.dirty = true;
        } else {
            // Đọc từ cache
            memcpy(data, hal_ctx.cache.data, STORAGE_MAX_SECTOR_SIZE);
        }
        return STATUS_SUCCESS;
    }

    // Cache miss, cần flush cache cũ nếu dirty
    if (hal_ctx.cache.valid && hal_ctx.cache.dirty) {
        int32_t ret = hal_write_sector(hal_ctx.cache.sector, hal_ctx.cache.data);
        if (ret != STATUS_SUCCESS) {
            return ret;
        }
    }

    // Đọc sector mới vào cache
    int32_t ret = hal_read_sector(sector, hal_ctx.cache.data);
    if (ret != STATUS_SUCCESS) {
        hal_ctx.cache.valid = false;
        return ret;
    }

    // Cập nhật cache
    hal_ctx.cache.sector = sector;
    hal_ctx.cache.valid = true;
    hal_ctx.cache.dirty = write;

    if (write) {
        memcpy(hal_ctx.cache.data, data, STORAGE_MAX_SECTOR_SIZE);
    } else {
        memcpy(data, hal_ctx.cache.data, STORAGE_MAX_SECTOR_SIZE);
    }

    return STATUS_SUCCESS;
}

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t hal_init(const hal_config_t *config)
{
    if (!config) {
        return STATUS_INVALID;
    }

    /* Khởi tạo IP driver */
    ip_config_t ip_config = {
        .file_path = config->file_path,
        .base_addr = config->base_addr,
        .irq_num = config->irq_num,
        .use_dma = config->use_dma
    };

    if (ip_driver_init(&ip_config) != IP_SUCCESS) {
        return STATUS_ERROR;
    }

    /* Khởi tạo context */
    memset(&hal_ctx, 0, sizeof(hal_context_t));
    memcpy(&hal_config, config, sizeof(hal_config_t));
    memset(&hal_status, 0, sizeof(hal_status_t));
    hal_status.is_initialized = true;

    hal_initialized = true;
    return STATUS_SUCCESS;
}

int32_t hal_read_sector(uint32_t sector, uint8_t *buffer)
{
    if (!hal_initialized || !buffer) {
        return STATUS_INVALID;
    }

    return (ip_read_sector(sector, buffer) == IP_SUCCESS) ? STATUS_SUCCESS : STATUS_ERROR;
}

int32_t hal_write_sector(uint32_t sector, const uint8_t *buffer)
{
    if (!hal_initialized || !buffer) {
        return STATUS_INVALID;
    }

    return (ip_write_sector(sector, buffer) == IP_SUCCESS) ? STATUS_SUCCESS : STATUS_ERROR;
}

int32_t hal_deinit(void)
{
    if (!hal_initialized) {
        return STATUS_SUCCESS;
    }

    /* Đóng file trong IP driver */
    if (ip_close() != IP_SUCCESS) {
        return STATUS_ERROR;
    }

    /* Reset context */
    memset(&hal_ctx, 0, sizeof(hal_context_t));
    memset(&hal_status, 0, sizeof(hal_status_t));
    hal_initialized = false;

    return STATUS_SUCCESS;
}

int32_t hal_read(void *buffer, uint32_t size, uint32_t timeout)
{
    if (!buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE || 
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    if (hal_status.is_busy) {
        return STATUS_BUSY;
    }

    hal_status.is_busy = true;
    hal_status.transfer_count++;

    // TODO: Implement actual read based on transfer mode

    hal_status.is_busy = false;
    return STATUS_SUCCESS;
}

int32_t hal_write(const void *buffer, uint32_t size, uint32_t timeout)
{
    if (!buffer || size == 0 || size > HAL_MAX_BUFFER_SIZE ||
        timeout < HAL_MIN_TIMEOUT || timeout > HAL_MAX_TIMEOUT) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    if (hal_status.is_busy) {
        return STATUS_BUSY;
    }

    hal_status.is_busy = true;
    hal_status.transfer_count++;

    // TODO: Implement actual write based on transfer mode

    hal_status.is_busy = false;
    return STATUS_SUCCESS;
}

int32_t hal_register_callback(void (*callback)(void *), uint32_t event_id)
{
    (void)event_id; // Unused parameter
    if (!callback) {
        return STATUS_INVALID_PARAMETER;
    }

    // Tìm slot trống
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (hal_ctx.callbacks[i].callback == NULL) {
            hal_ctx.callbacks[i].callback = callback;
            hal_ctx.callbacks[i].param = NULL;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ERROR;
}

int32_t hal_unregister_callback(uint32_t event_id)
{
    (void)event_id; // Unused parameter
    // Tìm callback cần xóa
    for (uint32_t i = 0; i < HAL_MAX_CALLBACKS; i++) {
        if (hal_ctx.callbacks[i].callback != NULL) {
            hal_ctx.callbacks[i].callback = NULL;
            hal_ctx.callbacks[i].param = NULL;
            return STATUS_SUCCESS;
        }
    }

    return STATUS_ERROR;
}

int32_t hal_set_transfer_mode(hal_transfer_mode_t mode)
{
    if (mode > HAL_MODE_DMA) {
        return STATUS_INVALID_PARAMETER;
    }

    transfer_mode = mode;
    return STATUS_SUCCESS;
}

int32_t hal_get_device_info(hal_device_info_t *info)
{
    if (!info) {
        return STATUS_INVALID_PARAMETER;
    }

    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    // TODO: Get actual device info
    info->device_id = 0;
    info->manufacturer_id = 0;
    info->version = 0;
    info->capabilities = 0;

    return STATUS_SUCCESS;
}

int32_t hal_get_status(hal_status_t *status)
{
    if (!status) {
        return STATUS_INVALID_PARAMETER;
    }

    memcpy(status, &hal_status, sizeof(hal_status_t));
    return STATUS_SUCCESS;
}

int32_t hal_reset(void)
{
    if (!hal_status.is_initialized) {
        return STATUS_NOT_READY;
    }

    // Reset status
    memset(&hal_status, 0, sizeof(hal_status_t));
    hal_status.is_initialized = true;

    // Reset transfer mode
    transfer_mode = HAL_MODE_POLLING;

    return STATUS_SUCCESS;
}

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/

