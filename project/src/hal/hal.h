/**
 * @file hal.h
 * @author Le Duc Son (sonld@hselab.com)
 * @date 2020-11-24
 * @brief HAL (Hardware Abstraction Layer) interface
 * @details This file contains the interface for HAL module
 */

#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include "../common/common_types.h"
#include "../ip_driver/ip_driver.h"
#include "wl_layer.h"

/**
 * Structure representing HAL
 */
typedef struct {
    /**
     * Pointer to IPDriver structure
     */
    IPDriver ip_driver;

    /**
     * Sector size
     */
    SectorSize sector_size;

    /**
     * ESP-IDF wear-levelling translation layer.
     * When wl.enabled is true, logical sectors are remapped to their
     * physical location inside the image (storage.bin style dumps).
     */
    WLLayer wl;
} HAL;

/**
 * Initialize HAL
 * @param hal Pointer to HAL structure
 * @param img_path Path to image file
 * @param sector_size Sector size
 * @return 0 if success, -1 if failed
 */
int hal_init(HAL* hal, const char* img_path, SectorSize sector_size);

/**
 * Deinitialize HAL
 * @param hal Pointer to HAL structure
 * @return 0 if success, -1 if failed
 */
int hal_deinit(HAL* hal);

/**
 * Read a sector from image file
 * @param hal Pointer to HAL structure
 * @param sector_number Sector number to read
 * @param buffer Buffer to store read data
 * @return Number of bytes read if success, -1 if failed
 */
int hal_read_sector(HAL* hal, uint32_t sector_number, void* buffer);

/**
 * Write a sector to image file
 * @param hal Pointer to HAL structure
 * @param sector_number Sector number to write
 * @param buffer Buffer containing data to write
 * @return Number of bytes written if success, -1 if failed
 */
int hal_write_sector(HAL* hal, uint32_t sector_number, const void* buffer);

/**
 * Close HAL
 * @param hal Pointer to HAL structure
 */
void hal_close(HAL* hal);

/**
 * Get sector size
 * @param hal Pointer to HAL structure
 * @return Sector size
 */
uint32_t hal_get_sector_size(HAL* hal);

#endif // HAL_H

