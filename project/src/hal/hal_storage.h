/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Header file cho HAL Storage module, định nghĩa các cấu trúc và
 *   hàm cần thiết cho việc quản lý storage driver.
 *********************************************************************/
#ifndef __HAL_STORAGE_H
#define __HAL_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../common/common_types.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Storage sector size */
#define STORAGE_MAX_SECTOR_SIZE   512

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* Storage driver interface */
typedef struct {
    int32_t (*init)(const void *config);
    int32_t (*deinit)(void);
    int32_t (*read_sector)(uint32_t sector, uint8_t *buffer);
    int32_t (*write_sector)(uint32_t sector, const uint8_t *buffer);
    int32_t (*get_info)(void *info);
} storage_driver_t;

#ifdef __cplusplus
}
#endif

#endif /* __HAL_STORAGE_H */

/*********************************************************************
 * UUID: 5a9c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
 *********************************************************************/ 