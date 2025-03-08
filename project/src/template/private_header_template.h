/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Định nghĩa private cho module.
 *   KHÔNG sử dụng trực tiếp các định nghĩa này từ bên ngoài module.
 *********************************************************************/
#ifndef __MODULE_NAME_PRIVATE_H
#define __MODULE_NAME_PRIVATE_H

#include "module_name.h"

/*********************************************************************
 * Private Macro Definitions
 *********************************************************************/

/* Module States */
#define MODULE_STATE_UNINITIALIZED  0U
#define MODULE_STATE_INITIALIZED    1U
#define MODULE_STATE_RUNNING       2U
#define MODULE_STATE_ERROR         3U

/* Debug Configuration */
#define MODULE_DEBUG_MODE          0U

/* Module Constants */
#define MODULE_MAX_RETRIES         3U
#define MODULE_TIMEOUT_MS          1000U
#define MODULE_BUFFER_SIZE         512U

/* Module Masks */
#define MODULE_STATUS_MASK        0x0FU
#define MODULE_ERROR_MASK         0xF0U

/*********************************************************************
 * Private Type Definitions
 *********************************************************************/

/* Module States */
typedef enum {
    INTERNAL_STATE_IDLE = 0,
    INTERNAL_STATE_BUSY,
    INTERNAL_STATE_ERROR
} internal_state_t;

/* Module Context */
typedef struct {
    uint8_t state;
    module_config_t config;
    internal_state_t internal_state;
    uint32_t error_count;
    uint32_t last_access;
    uint8_t buffer[MODULE_BUFFER_SIZE];
} module_context_t;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Kiểm tra điều kiện
 *
 * @param param Tham số cần kiểm tra
 * @return true nếu hợp lệ, false nếu không hợp lệ
 */
static bool module_check_condition(uint32_t param);

/**
 * @brief Xử lý lỗi
 *
 * @param error_code Mã lỗi
 * @return MODULE_SUCCESS nếu có thể khôi phục, mã lỗi nếu không
 */
static int32_t module_handle_error(int32_t error_code);

#endif /* __MODULE_NAME_PRIVATE_H */ 