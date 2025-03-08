/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Định nghĩa private cho module này.
 *   KHÔNG sử dụng trực tiếp các định nghĩa này từ bên ngoài module.
 *********************************************************************/
#ifndef __MODULE_NAME_PRIVATE_H
#define __MODULE_NAME_PRIVATE_H

#include "module_name.h"

/*********************************************************************
 * Private Macro Definitions
 *********************************************************************/

/* Module States */
#define MODULE_STATE_ON   1U
#define MODULE_STATE_OFF  0U

/* Debug Configuration */
#define MODULE_DEBUG_MODE MODULE_STATE_OFF

/* Module Constants */
#define PRIVATE_CONSTANT_1  100U
#define PRIVATE_CONSTANT_2  200U

/* Module Masks */
#define PRIVATE_MASK_1    0x0FU
#define PRIVATE_MASK_2    0xF0U

/*********************************************************************
 * Private Type Definitions
 *********************************************************************/

/* Module States */
typedef enum {
    STATE_INIT = 0,
    STATE_READY,
    STATE_RUNNING,
    STATE_ERROR
} module_state_t;

/* Module Configuration */
typedef struct {
    uint32_t config1;    /* Configuration parameter 1 */
    uint16_t config2;    /* Configuration parameter 2 */
    uint8_t flags;       /* Configuration flags */
} module_config_t;

/* Module Context */
typedef struct {
    module_state_t state;     /* Current state */
    module_config_t config;   /* Configuration */
    uint32_t status;         /* Status flags */
} module_context_t;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Initialize module context
 *
 * @param context Pointer to module context
 * @return Status code
 */
int32_t module_init_context(module_context_t *context);

/**
 * @brief Process module state
 *
 * @param context Pointer to module context
 * @return Status code
 */
int32_t module_process_state(module_context_t *context);

#endif /* __MODULE_NAME_PRIVATE_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/ 