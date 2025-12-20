/**
 * @file ip_driver.h
 * @author Le Duc Son (sonld@hselab.com)
 * @date 2020-11-24
 * @brief IP Driver interface
 * @details This file contains the interface for IP Driver module
 */

#ifndef IP_DRIVER_H
#define IP_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "../common/common_types.h"

/**
 * IP Driver structure
 */
typedef struct {
    FILE* img_file; /**< File pointer to image file */
    uint32_t buffer_size; /**< Buffer size to read/write data */
} IPDriver;

/**
 * Initialize IP Driver
 * @param driver Pointer to IPDriver structure
 * @param img_path Path to image file
 * @return 0 if success, -1 if failed
 */
int ip_driver_init(IPDriver* driver, const char* img_path);

/**
 * Read a sector from image file
 * @param driver Pointer to IPDriver structure
 * @param offset Sector number to read
 * @param buffer Buffer to store read data
 * @return Number of bytes read if success, -1 if failed
 */
int ip_driver_read_buffer(IPDriver* driver, uint32_t offset, void* buffer);

/**
 * Write a sector to image file
 * @param driver Pointer to IPDriver structure
 * @param offset Sector number to write
 * @param buffer Buffer containing data to write
 * @return Number of bytes written if success, -1 if failed
 */
int ip_driver_write_buffer(IPDriver* driver, uint32_t offset, const void* buffer);

/**
 * Close IP Driver
 * @param driver Pointer to IPDriver structure
 */
void ip_driver_close(IPDriver* driver);

#endif

