/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Định nghĩa private cho HAL module.
 *   KHÔNG sử dụng trực tiếp các định nghĩa này từ bên ngoài module.
 *********************************************************************/
#ifndef __HAL_PRIVATE_H
#define __HAL_PRIVATE_H

#include "hal.h"

/*********************************************************************
 * Private Macro Definitions
 *********************************************************************/

/* Module States */
#define HAL_STATE_UNINITIALIZED  0U
#define HAL_STATE_INITIALIZED    1U
#define HAL_STATE_RUNNING       2U
#define HAL_STATE_ERROR         3U

/* Debug Configuration */
#define HAL_DEBUG_MODE          0U

/* Hardware Constants */
#define HAL_REG_BASE_ADDR      0x40000000U
#define HAL_REG_STATUS         0x00U
#define HAL_REG_CONTROL        0x04U
#define HAL_REG_DATA           0x08U
#define HAL_REG_INT_ENABLE     0x0CU
#define HAL_REG_INT_STATUS     0x10U

/* Register Bits */
#define HAL_STATUS_BUSY        (1U << 0)
#define HAL_STATUS_ERROR       (1U << 1)
#define HAL_STATUS_READY       (1U << 2)
#define HAL_CONTROL_START      (1U << 0)
#define HAL_CONTROL_STOP       (1U << 1)
#define HAL_CONTROL_RESET      (1U << 2)
#define HAL_INT_RX_READY       (1U << 0)
#define HAL_INT_TX_EMPTY       (1U << 1)
#define HAL_INT_ERROR          (1U << 2)

/* Buffer Management */
#define HAL_RX_BUFFER_SIZE    256U
#define HAL_TX_BUFFER_SIZE    256U
#define HAL_FIFO_SIZE         16U
#define HAL_MAX_CALLBACKS     8U

/* Callback Structure */
typedef struct {
    void (*callback)(void *);  /* Callback function */
    void *param;               /* Callback parameter */
    uint32_t event_id;        /* Event ID */
} hal_callback_t;

/*********************************************************************
 * Private Type Definitions
 *********************************************************************/

/* Register Map */
typedef struct {
    volatile uint32_t status;      /* Trạng thái */
    volatile uint32_t control;     /* Điều khiển */
    volatile uint32_t data;        /* Dữ liệu */
    volatile uint32_t int_enable;  /* Cho phép ngắt */
    volatile uint32_t int_status;  /* Trạng thái ngắt */
} hal_reg_map_t;

/* Ring Buffer */
typedef struct {
    uint8_t *buffer;              /* Con trỏ buffer */
    uint32_t size;                /* Kích thước buffer */
    uint32_t head;                /* Vị trí đầu */
    uint32_t tail;                /* Vị trí cuối */
    uint32_t count;               /* Số phần tử */
} hal_ring_buffer_t;

/* HAL Context */
typedef struct {
    uint8_t state;                /* Trạng thái module */
    hal_config_t config;          /* Cấu hình */
    hal_reg_map_t *reg;          /* Con trỏ đến registers */
    hal_ring_buffer_t rx_buffer;  /* Buffer nhận */
    hal_ring_buffer_t tx_buffer;  /* Buffer gửi */
    hal_transfer_mode_t mode;     /* Chế độ truyền */
    hal_callback_t callbacks[HAL_MAX_CALLBACKS]; /* Mảng callbacks */
    uint32_t error_count;         /* Số lỗi */
    uint32_t transfer_count;      /* Số lần truyền */
} hal_context_t;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo ring buffer
 *
 * @param buffer Con trỏ đến ring buffer
 * @param size Kích thước buffer
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t hal_init_ring_buffer(hal_ring_buffer_t *buffer, uint32_t size);

/**
 * @brief Ghi dữ liệu vào ring buffer
 *
 * @param buffer Con trỏ đến ring buffer
 * @param data Con trỏ đến dữ liệu
 * @param size Số byte cần ghi
 * @return Số byte đã ghi
 */
static uint32_t hal_write_ring_buffer(hal_ring_buffer_t *buffer, 
                                    const uint8_t *data, 
                                    uint32_t size);

/**
 * @brief Đọc dữ liệu từ ring buffer
 *
 * @param buffer Con trỏ đến ring buffer
 * @param data Con trỏ đến buffer lưu dữ liệu
 * @param size Số byte cần đọc
 * @return Số byte đã đọc
 */
static uint32_t hal_read_ring_buffer(hal_ring_buffer_t *buffer,
                                   uint8_t *data,
                                   uint32_t size);

/**
 * @brief Xử lý ngắt
 */
static void hal_irq_handler(void);

#endif /* __HAL_PRIVATE_H */ 