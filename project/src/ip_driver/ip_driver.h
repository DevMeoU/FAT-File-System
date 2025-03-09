/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Interface cho IP Storage module, cung cấp các hàm để truy cập
 *   thiết bị lưu trữ qua giao thức IP. Module này đóng vai trò là
 *   một storage driver cụ thể trong hệ thống.
 *********************************************************************/
#ifndef __IP_DRIVER_H
#define __IP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../common/common_types.h"

/*********************************************************************
 * Constants
 *********************************************************************/

/* Status codes */
#define IP_SUCCESS          0
#define IP_ERROR           -1
#define IP_TIMEOUT         -2
#define IP_INVALID_PARAM   -3

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* IP driver configuration */
typedef struct {
    const char *file_path;  /* Path to storage file */
    uint32_t base_addr;     /* Base address */
    uint32_t irq_num;       /* IRQ number */
    bool use_dma;          /* Use DMA */
} ip_config_t;

/*********************************************************************
 * Public Functions
 *********************************************************************/

/**
 * @brief Initialize IP driver
 * 
 * @param config Configuration parameters including storage file path
 * @return IP_SUCCESS if successful, error code otherwise
 */
int32_t ip_driver_init(const ip_config_t *config);

/**
 * @brief Close IP driver and release resources
 * 
 * @return IP_SUCCESS if successful, error code otherwise
 */
int32_t ip_close(void);

/**
 * @brief Read sector from IP
 * 
 * @param sector Sector number
 * @param buffer Buffer to store data
 * @return IP_SUCCESS if successful, error code otherwise
 */
int32_t ip_read_sector(uint32_t sector, uint8_t *buffer);

/**
 * @brief Write sector to IP
 * 
 * @param sector Sector number
 * @param buffer Data to write
 * @return IP_SUCCESS if successful, error code otherwise
 */
int32_t ip_write_sector(uint32_t sector, const uint8_t *buffer);

#ifdef __cplusplus
}
#endif

#endif /* __IP_DRIVER_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/

