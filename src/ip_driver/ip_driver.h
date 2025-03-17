/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   File header chứa các định nghĩa và khai báo public của module IP driver.
 *   Các định nghĩa và khai báo này được sử dụng bởi các module khác.
 *********************************************************************/

#ifndef IP_DRIVER_H
#define IP_DRIVER_H

/*********************************************************************
 * Include Files
 *********************************************************************/
#include "../common/common_types.h"

/*********************************************************************
 * Public Definitions
 *********************************************************************/
#define IP_MAX_BUFFER_SIZE 4096

/* Error codes */
#define IP_SUCCESS 0
#define IP_ERROR_INVALID -1
#define IP_ERROR_IO -2
#define IP_ERROR_NOT_FOUND -3
#define IP_ERROR_ACCESS_DENIED -4
#define IP_ERROR_ALREADY_EXISTS -5
#define IP_ERROR_NOT_EMPTY -6
#define IP_ERROR_DISK_FULL -7
#define IP_ERROR_NOT_READY -8
#define IP_ERROR_BUSY -9

/*********************************************************************
 * Public Types
 *********************************************************************/
typedef enum {
    IP_MODE_POLLING = 0,
    IP_MODE_INTERRUPT = 1,
    IP_MODE_DMA = 2
} ip_transfer_mode_t;

typedef struct {
    ip_transfer_mode_t mode;
    uint32_t sector_size;
    uint32_t total_sectors;
} ip_config_t;

typedef struct {
    uint32_t device_id;
    uint32_t manufacturer_id;
    uint32_t version;
    uint32_t capabilities;
} ip_device_info_t;

typedef struct {
    FILE *fp;
    char *img_path;
    uint32_t sector_size;
    uint32_t total_sectors;
} IpControl;

/*********************************************************************
 * Public Function Declarations
 *********************************************************************/
int32_t ip_driver_init(ip_config_t *config);
int32_t ip_driver_deinit(void);
int32_t ip_driver_read_sector(uint32_t sector_num, uint8_t *buffer);
int32_t ip_driver_write_sector(uint32_t sector_num, const uint8_t *buffer);
int32_t ip_driver_get_device_info(ip_device_info_t *info);
int32_t ip_driver_reset(void);

#endif /* IP_DRIVER_H */

/*********************************************************************
 * UUID: 3f8d2e1c-9b4a-4e85-8c6d-f7b2e3a1d5c9
 *********************************************************************/
