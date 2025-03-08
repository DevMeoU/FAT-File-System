/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Mô tả chức năng của module này và các interface public
 *   mà module cung cấp cho các module khác sử dụng.
 *********************************************************************/
#ifndef __MODULE_NAME_H
#define __MODULE_NAME_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Constants */
#define MODULE_CONSTANT_1    100U
#define MODULE_CONSTANT_2    200U

/* Status codes */
#define MODULE_SUCCESS          0x00
#define MODULE_ERROR           -1
#define MODULE_TIMEOUT         -2
#define MODULE_INVALID_PARAM   -3

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Enums */
typedef enum {
    MODULE_STATE_1 = 0,
    MODULE_STATE_2,
    MODULE_STATE_MAX
} module_state_t;

/* Structures */
typedef struct {
    uint32_t param1;    /* Mô tả param1 */
    uint8_t param2;     /* Mô tả param2 */
    bool enabled;       /* Trạng thái enable */
} module_config_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo module
 *
 * @param config Con trỏ đến cấu hình
 * @return MODULE_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t module_init(const module_config_t *config);

/**
 * @brief Hàm xử lý 1 của module
 *
 * @param param Tham số đầu vào
 * @return MODULE_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t module_function1(uint32_t param);

/**
 * @brief Hàm xử lý 2 của module
 *
 * @param data Con trỏ đến dữ liệu
 * @param size Kích thước dữ liệu
 * @return MODULE_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t module_function2(const uint8_t *data, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_NAME_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/ 