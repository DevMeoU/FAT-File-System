/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Public header cho IP Storage module, định nghĩa các cấu trúc và
 *   hàm public cho module IP Storage.
 *********************************************************************/
#ifndef __IP_STORAGE_H
#define __IP_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ip_driver.h"

/*********************************************************************
 * Constants
 *********************************************************************/
#define IP_SECTOR_SIZE         512
#define IP_MAX_SECTORS         0x1000000

/* Status codes */
#define IP_STATUS_SUCCESS      0
#define IP_STATUS_ERROR       -1
#define IP_STATUS_TIMEOUT     -2
#define IP_STATUS_NO_MEDIA    -3

/* IP address structure */
typedef struct {
    uint8_t bytes[4];
} ip_addr_t;

#ifdef __cplusplus
}
#endif

#endif /* __IP_STORAGE_H */ 