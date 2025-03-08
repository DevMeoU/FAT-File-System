/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module status định nghĩa các mã trạng thái và lỗi được sử dụng
 *   trong toàn bộ hệ thống.
 *********************************************************************/
#ifndef __MODULE_STATUS_H
#define __MODULE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Status code ranges */
#define STATUS_SUCCESS              0x00000000
#define STATUS_WARNING_BASE         0x10000000
#define STATUS_ERROR_BASE          0x20000000
#define STATUS_FATAL_BASE          0x30000000

/* Common status codes */
#define STATUS_OK                   STATUS_SUCCESS
#define STATUS_PENDING             (STATUS_SUCCESS + 1)
#define STATUS_TIMEOUT             (STATUS_WARNING_BASE + 0)
#define STATUS_NO_MEMORY           (STATUS_ERROR_BASE + 0)
#define STATUS_INVALID_PARAM       (STATUS_ERROR_BASE + 1)
#define STATUS_NOT_INITIALIZED     (STATUS_ERROR_BASE + 2)
#define STATUS_NOT_SUPPORTED       (STATUS_ERROR_BASE + 3)
#define STATUS_BUSY               (STATUS_ERROR_BASE + 4)
#define STATUS_IO_ERROR           (STATUS_ERROR_BASE + 5)
#define STATUS_SYSTEM_ERROR       (STATUS_FATAL_BASE + 0)

/* Module specific status codes */
#define MODULE_STATUS_BASE         0x01000000
#define MODULE_ERROR_1            (MODULE_STATUS_BASE + 1)
#define MODULE_ERROR_2            (MODULE_STATUS_BASE + 2)
#define MODULE_ERROR_3            (MODULE_STATUS_BASE + 3)

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Status code type */
typedef uint32_t status_t;

/* Error info structure */
typedef struct {
    status_t code;           /* Mã lỗi */
    const char *message;     /* Thông báo lỗi */
    const char *file;        /* File xảy ra lỗi */
    uint32_t line;          /* Dòng xảy ra lỗi */
} error_info_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Chuyển đổi mã lỗi thành chuỗi
 *
 * @param status Mã trạng thái
 * @return Chuỗi mô tả trạng thái
 */
const char *status_to_string(status_t status);

/**
 * @brief Kiểm tra xem mã trạng thái có phải là lỗi không
 *
 * @param status Mã trạng thái
 * @return true nếu là lỗi, false nếu không phải
 */
bool status_is_error(status_t status);

/**
 * @brief Lấy thông tin chi tiết về lỗi
 *
 * @param status Mã trạng thái
 * @param info Con trỏ đến struct lưu thông tin lỗi
 */
void status_get_error_info(status_t status, error_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* __MODULE_STATUS_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/ 