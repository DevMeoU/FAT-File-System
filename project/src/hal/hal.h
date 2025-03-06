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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h> // Include for size_t

/*********************************************************************
 * Function prototypes
 *********************************************************************/
/**
 * @brief Initializes the HAL layer with the given image path.
 *
 * @param img_path The path to the image file.
 * @return 0 on success, -1 on failure.
 */
int hal_init(const char *img_path);

/**
 * @brief Reads a sector into the provided buffer.
 *
 * @param sector The sector number to read.
 * @param buffer The buffer where the data will be stored.
 * @return 0 on success, -1 on failure.
 */
int hal_read_sector(unsigned int sector, unsigned char *buffer);

/**
 * @brief Writes data from the buffer into a sector.
 *
 * @param sector The sector number to write to.
 * @param buffer The buffer containing the data to write.
 * @return 0 on success, -1 on failure.
 */
int hal_write_sector(unsigned int sector, const unsigned char *buffer);

/**
 * @brief Configures the HAL layer with sector size and max sectors.
 *
 * @param sector_size The size of each sector.
 * @param max_sectors The maximum number of sectors, 0 for unlimited.
 */
void hal_configure(size_t sector_size, unsigned int max_sectors);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/