/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#ifndef HAL_H
#define HAL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Define
 *********************************************************************/
#define DEFAULT_SECTOR_SIZE 512

/*********************************************************************
 * Include
 *********************************************************************/
#include <stddef.h>

/*********************************************************************
 * Function prototypes
 *********************************************************************/
int hal_init(const char *img_path);
int hal_read_sector(unsigned int sector, unsigned char *buffer);
int hal_write_sector(unsigned int sector, const unsigned char *buffer);
void hal_configure(size_t sector_size, unsigned int max_sectors);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/