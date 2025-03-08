/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Implementation của HAL module, cung cấp các hàm để tương tác với
 *   phần cứng thông qua các registers và buffers.
 *********************************************************************/

/*********************************************************************
 * Include Files
 *********************************************************************/
#include "hal.h"
#include "hal_private.h"

/*********************************************************************
 * Private Variables
 *********************************************************************/
static hal_context_t hal_ctx = {0};
static uint8_t rx_buffer_data[HAL_RX_BUFFER_SIZE];
static uint8_t tx_buffer_data[HAL_TX_BUFFER_SIZE];

/*********************************************************************
 * Private Function Implementations
 *********************************************************************/

static int32_t hal_init_ring_buffer(hal_ring_buffer_t *buffer, uint32_t size) {
    // Kiểm tra tham số đầu vào
    if (!buffer || size == 0) {
        return HAL_INVALID_PARAM;
    }

    // Cấp phát bộ nhớ cho buffer
    buffer->buffer = (uint8_t *)malloc(size);
    if (!buffer->buffer) {
        return HAL_ERROR;
    }

    // Khởi tạo các thông số
    buffer->size = size;
    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;

    return HAL_SUCCESS;
}

static uint32_t hal_write_ring_buffer(hal_ring_buffer_t *buffer,
                                    const uint8_t *data,
                                    uint32_t size) {
    uint32_t written = 0;

    // Kiểm tra tham số đầu vào
    if (!buffer || !data || size == 0) {
        return 0;
    }

    // Ghi dữ liệu vào buffer
    while (written < size && buffer->count < buffer->size) {
        buffer->buffer[buffer->tail] = data[written++];
        buffer->tail = (buffer->tail + 1) % buffer->size;
        buffer->count++;
    }

    return written;
}

static uint32_t hal_read_ring_buffer(hal_ring_buffer_t *buffer,
                                   uint8_t *data,
                                   uint32_t size) {
    uint32_t read = 0;

    // Kiểm tra tham số đầu vào
    if (!buffer || !data || size == 0) {
        return 0;
    }

    // Đọc dữ liệu từ buffer
    while (read < size && buffer->count > 0) {
        data[read++] = buffer->buffer[buffer->head];
        buffer->head = (buffer->head + 1) % buffer->size;
        buffer->count--;
    }

    return read;
}

static void hal_irq_handler(void) {
    // Kiểm tra trạng thái ngắt
    uint32_t int_status = hal_ctx.reg->int_status;

    // Xử lý ngắt nhận
    if (int_status & HAL_INT_RX_READY) {
        uint8_t data = (uint8_t)hal_ctx.reg->data;
        hal_write_ring_buffer(&hal_ctx.rx_buffer, &data, 1);
    }

    // Xử lý ngắt gửi
    if (int_status & HAL_INT_TX_EMPTY) {
        uint8_t data;
        if (hal_read_ring_buffer(&hal_ctx.tx_buffer, &data, 1) > 0) {
            hal_ctx.reg->data = data;
        }
    }

    // Xử lý lỗi
    if (int_status & HAL_INT_ERROR) {
        hal_ctx.error_count++;
    }

    // Xóa cờ ngắt
    hal_ctx.reg->int_status = int_status;
}

/*********************************************************************
 * Public Function Implementations
 *********************************************************************/

int32_t hal_init(const hal_config_t *config) {
    // Kiểm tra tham số đầu vào
    if (!config) {
        return HAL_INVALID_PARAM;
    }

    // Kiểm tra các thông số cấu hình
    if (config->buffer_size < HAL_MIN_BUFFER_SIZE ||
        config->buffer_size > HAL_MAX_BUFFER_SIZE ||
        config->timeout < HAL_MIN_TIMEOUT ||
        config->timeout > HAL_MAX_TIMEOUT) {
        return HAL_INVALID_PARAM;
    }

    // Khởi tạo context
    memset(&hal_ctx, 0, sizeof(hal_ctx));
    memcpy(&hal_ctx.config, config, sizeof(hal_config_t));

    // Map registers
    hal_ctx.reg = (hal_reg_map_t *)HAL_REG_BASE_ADDR;

    // Khởi tạo buffers
    hal_ctx.rx_buffer.buffer = rx_buffer_data;
    hal_ctx.rx_buffer.size = HAL_RX_BUFFER_SIZE;
    hal_ctx.tx_buffer.buffer = tx_buffer_data;
    hal_ctx.tx_buffer.size = HAL_TX_BUFFER_SIZE;

    // Reset thiết bị
    hal_ctx.reg->control = HAL_CONTROL_RESET;
    while (hal_ctx.reg->status & HAL_STATUS_BUSY);

    // Cấu hình ngắt
    if (config->interrupt_enable) {
        hal_ctx.reg->int_enable = HAL_INT_RX_READY | 
                                 HAL_INT_TX_EMPTY | 
                                 HAL_INT_ERROR;
    }

    // Cập nhật trạng thái
    hal_ctx.state = HAL_STATE_INITIALIZED;
    
    return HAL_SUCCESS;
}

int32_t hal_read(void *buffer, uint32_t size, uint32_t timeout) {
    uint32_t bytes_read = 0;
    uint32_t start_time = 0; // TODO: Get current time

    // Kiểm tra tham số đầu vào
    if (!buffer || size == 0) {
        return HAL_INVALID_PARAM;
    }

    // Kiểm tra trạng thái
    if (hal_ctx.state != HAL_STATE_INITIALIZED) {
        return HAL_ERROR;
    }

    // Đọc dữ liệu theo chế độ
    switch (hal_ctx.mode) {
        case HAL_MODE_POLLING:
            while (bytes_read < size) {
                if (hal_ctx.reg->status & HAL_STATUS_READY) {
                    ((uint8_t *)buffer)[bytes_read++] = (uint8_t)hal_ctx.reg->data;
                }
                // TODO: Check timeout
            }
            break;

        case HAL_MODE_INTERRUPT:
            bytes_read = hal_read_ring_buffer(&hal_ctx.rx_buffer,
                                            buffer,
                                            size);
            break;

        case HAL_MODE_DMA:
            // TODO: Implement DMA transfer
            break;

        default:
            return HAL_ERROR;
    }

    hal_ctx.transfer_count++;
    return (bytes_read > 0) ? HAL_SUCCESS : HAL_TIMEOUT;
}

int32_t hal_write(const void *buffer, uint32_t size, uint32_t timeout) {
    uint32_t bytes_written = 0;
    uint32_t start_time = 0; // TODO: Get current time

    // Kiểm tra tham số đầu vào
    if (!buffer || size == 0) {
        return HAL_INVALID_PARAM;
    }

    // Kiểm tra trạng thái
    if (hal_ctx.state != HAL_STATE_INITIALIZED) {
        return HAL_ERROR;
    }

    // Ghi dữ liệu theo chế độ
    switch (hal_ctx.mode) {
        case HAL_MODE_POLLING:
            while (bytes_written < size) {
                if (!(hal_ctx.reg->status & HAL_STATUS_BUSY)) {
                    hal_ctx.reg->data = ((uint8_t *)buffer)[bytes_written++];
                }
                // TODO: Check timeout
            }
            break;

        case HAL_MODE_INTERRUPT:
            bytes_written = hal_write_ring_buffer(&hal_ctx.tx_buffer,
                                                buffer,
                                                size);
            break;

        case HAL_MODE_DMA:
            // TODO: Implement DMA transfer
            break;

        default:
            return HAL_ERROR;
    }

    hal_ctx.transfer_count++;
    return (bytes_written > 0) ? HAL_SUCCESS : HAL_TIMEOUT;
}

int32_t hal_set_transfer_mode(hal_transfer_mode_t mode) {
    // Kiểm tra tham số đầu vào
    if (mode > HAL_MODE_DMA) {
        return HAL_INVALID_PARAM;
    }

    // Kiểm tra trạng thái
    if (hal_ctx.state != HAL_STATE_INITIALIZED) {
        return HAL_ERROR;
    }

    // Cập nhật chế độ
    hal_ctx.mode = mode;

    return HAL_SUCCESS;
}

int32_t hal_get_device_info(hal_device_info_t *info) {
    // Kiểm tra tham số đầu vào
    if (!info) {
        return HAL_INVALID_PARAM;
    }

    // TODO: Read device information from registers
    info->device_id = 0;
    info->manufacturer_id = 0;
    info->version = 0;
    info->capabilities = 0;

    return HAL_SUCCESS;
}

int32_t hal_get_status(hal_status_t *status) {
    // Kiểm tra tham số đầu vào
    if (!status) {
        return HAL_INVALID_PARAM;
    }

    // Cập nhật trạng thái
    status->is_initialized = (hal_ctx.state == HAL_STATE_INITIALIZED);
    status->is_busy = (hal_ctx.reg->status & HAL_STATUS_BUSY) != 0;
    status->error_count = hal_ctx.error_count;
    status->transfer_count = hal_ctx.transfer_count;

    return HAL_SUCCESS;
}

int32_t hal_reset(void) {
    // Kiểm tra trạng thái
    if (hal_ctx.state != HAL_STATE_INITIALIZED) {
        return HAL_ERROR;
    }

    // Reset thiết bị
    hal_ctx.reg->control = HAL_CONTROL_RESET;
    while (hal_ctx.reg->status & HAL_STATUS_BUSY);

    // Reset các biến đếm
    hal_ctx.error_count = 0;
    hal_ctx.transfer_count = 0;

    return HAL_SUCCESS;
}

int32_t hal_register_callback(void (*callback)(void *), uint32_t event_id) {
    // Kiểm tra tham số đầu vào
    if (!callback) {
        return HAL_INVALID_PARAM;
    }

    // Kiểm tra trạng thái
    if (hal_ctx.state != HAL_STATE_INITIALIZED) {
        return HAL_ERROR;
    }

    // Đăng ký callback
    hal_ctx.callback = callback;

    return HAL_SUCCESS;
}

/*********************************************************************
 * End of File
 *********************************************************************/
