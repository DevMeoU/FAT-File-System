/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/

/*********************************************************************
 * Include
 *********************************************************************/
#include "hal.h"
#include "ip_driver.h"

/*********************************************************************
 * Define
 *********************************************************************/
#define SECTOR_SIZE 512

/*********************************************************************
 * Function prototypes
 *********************************************************************/

/*********************************************************************
 * Implementations
 *********************************************************************/
int hal_init(const char *img_path) {
    return ip_driver_init(img_path);
}

int hal_read_sector(unsigned int sector, unsigned char *buffer) {
    unsigned int offset = sector * SECTOR_SIZE;
    int result = ip_driver_read(offset, buffer, SECTOR_SIZE);
    return result == 0 ? 0 : -1;  /* Return 0 on success, -1 on error */
}

int hal_write_sector(unsigned int sector, const unsigned char *buffer) {
    unsigned int offset = sector * SECTOR_SIZE;
    int result = ip_driver_write(offset, buffer, SECTOR_SIZE);
    return result == 0 ? 0 : -1;  /* Return 0 on success, -1 on error */
}

void hal_configure(size_t sector_size, unsigned int max_sectors) {
    /* Configure parameters like sector size or maximum number of sectors
       Currently, sector size is defined as a constant SECTOR_SIZE */
}

