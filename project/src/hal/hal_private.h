/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Private header cho HAL Storage module, định nghĩa các cấu trúc và
 *   hàm nội bộ chỉ sử dụng trong module. KHÔNG export các định nghĩa
 *   này ra bên ngoài module.
 *********************************************************************/
#ifndef __HAL_STORAGE_PRIVATE_H
#define __HAL_STORAGE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_storage.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Module states */
#define HAL_STATE_UNINITIALIZED   0
#define HAL_STATE_INITIALIZED     1
#define HAL_STATE_ERROR          2

/* Debug configurations */
#define HAL_DEBUG_ENABLED        1
#define HAL_DEBUG_LEVEL          2

/* Buffer configurations */
#define HAL_RX_BUFFER_SIZE       2048
#define HAL_TX_BUFFER_SIZE       2048
#define HAL_CACHE_SIZE          16

/* Hardware configurations */
#define HAL_REG_BASE_ADDR       0x40000000
#define HAL_INT_RX_READY        0x01
#define HAL_INT_TX_READY        0x02
#define HAL_INT_ERROR           0x04

/* Control register bits */
#define HAL_CONTROL_RESET       0x01
#define HAL_CONTROL_ENABLE      0x02
#define HAL_CONTROL_DMA         0x04

/* Status register bits */
#define HAL_STATUS_BUSY         0x01
#define HAL_STATUS_ERROR        0x02
#define HAL_STATUS_READY        0x04

/* Maximum number of callbacks */
#define HAL_MAX_CALLBACKS       8

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Ring buffer structure */
typedef struct {
    uint8_t *buffer;    /* Buffer pointer */
    uint32_t size;      /* Buffer size */
    uint32_t head;      /* Read index */
    uint32_t tail;      /* Write index */
    uint32_t count;     /* Number of bytes */
} hal_ring_buffer_t;

/* Callback structure */
typedef struct {
    callback_t callback;  /* Callback function */
    void *param;         /* Callback parameter */
    uint32_t event_id;   /* Event identifier */
} hal_callback_t;

/* Hardware register map */
typedef struct {
    volatile uint32_t control;      /* Control register */
    volatile uint32_t status;       /* Status register */
    volatile uint32_t int_enable;   /* Interrupt enable */
    volatile uint32_t int_status;   /* Interrupt status */
    volatile uint32_t data;         /* Data register */
    volatile uint32_t dma_addr;     /* DMA address */
    volatile uint32_t dma_count;    /* DMA count */
    volatile uint32_t reserved[9];  /* Reserved */
} hal_reg_map_t;

/* HAL context structure */
typedef struct {
    uint8_t state;                  /* Module state */
    hal_config_t config;            /* Configuration */
    hal_reg_map_t *reg;            /* Register map */
    hal_ring_buffer_t rx_buffer;    /* Receive buffer */
    hal_ring_buffer_t tx_buffer;    /* Transmit buffer */
    hal_callback_t callbacks[HAL_MAX_CALLBACKS]; /* Callbacks */
    uint32_t error_count;           /* Error counter */
    uint32_t transfer_count;        /* Transfer counter */
    
    /* Storage management */
    storage_driver_t *current_storage;  /* Current storage driver */
    uint32_t sector_size;              /* Current sector size */
    uint32_t sector_count;             /* Current sector count */
    
    /* Cache management */
    struct {
        uint32_t sector;                /* Sector number */
        uint8_t data[STORAGE_MAX_SECTOR_SIZE]; /* Sector data */
        bool valid;                     /* Cache valid */
        bool dirty;                     /* Cache modified */
    } cache;
    
    /* DMA management */
    void *dma_buffer;                  /* DMA buffer */
    uint32_t dma_size;                 /* DMA buffer size */
    bool dma_busy;                     /* DMA busy flag */
} hal_context_t;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo ring buffer
 * 
 * @param buffer Ring buffer structure
 * @param size Buffer size
 * @return HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t hal_init_ring_buffer(hal_ring_buffer_t *buffer, uint32_t size);

/**
 * @brief Ghi dữ liệu vào ring buffer
 * 
 * @param buffer Ring buffer structure
 * @param data Data to write
 * @param size Data size
 * @return Số byte đã ghi
 */
static uint32_t hal_write_ring_buffer(hal_ring_buffer_t *buffer,
                                    const uint8_t *data,
                                    uint32_t size);

/**
 * @brief Đọc dữ liệu từ ring buffer
 * 
 * @param buffer Ring buffer structure
 * @param data Buffer to store data
 * @param size Maximum size to read
 * @return Số byte đã đọc
 */
static uint32_t hal_read_ring_buffer(hal_ring_buffer_t *buffer,
                                   uint8_t *data,
                                   uint32_t size);

/**
 * @brief Xử lý ngắt
 */
static void hal_irq_handler(void);

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
                              bool write);

#ifdef __cplusplus
}
#endif

#endif /* __HAL_STORAGE_PRIVATE_H */ 