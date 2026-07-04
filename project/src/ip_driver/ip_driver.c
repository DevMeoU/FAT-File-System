/**
 * @file ip_driver.c
 * @author Le Duc Son (sonld@hselab.com)
 * @date 2020-11-24
 * @brief IP Driver implementation
 */

#include "ip_driver.h"
#include "ip_driver_private.h"
#include <string.h>

/**
 * Initialize IP Driver
 * @param driver Pointer to IPDriver structure
 * @param img_path Path to image file
 * @return 0 if success, -1 if failed
 */
int ip_driver_init(IPDriver* driver, const char* img_path) {
    if (!driver || !img_path) return -1;

    /* Accept plain disk images (.img) and raw flash partition dumps (.bin,
     * e.g. ESP-IDF storage.bin) */
    const char* ext = strrchr(img_path, '.');
    if (!ext || (strcmp(ext, ".img") != 0 && strcmp(ext, ".bin") != 0)) return -1;

    driver->img_file = fopen(img_path, "rb+");
    if (!driver->img_file) {
        /* Fall back to read-only when the file is write-protected */
        driver->img_file = fopen(img_path, "rb");
    }
    if (!driver->img_file) return -1;

    return 0;
}

/**
 * Read a sector from image file
 * @param driver Pointer to IPDriver structure
 * @param offset Sector number to read
 * @param buffer Buffer to store read data
 * @return Number of bytes read if success, -1 if failed
 */
int ip_driver_read_buffer(IPDriver* driver, uint32_t offset, void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    if (fseek(driver->img_file, offset * driver->buffer_size, SEEK_SET) != 0) {
        return -1;
    }
    return fread(buffer, 1, driver->buffer_size, driver->img_file);
}

/**
 * Write a sector to image file
 * @param driver Pointer to IPDriver structure
 * @param offset Sector number to write
 * @param buffer Buffer containing data to write
 * @return Number of bytes written if success, -1 if failed
 */
int ip_driver_write_buffer(IPDriver* driver, uint32_t offset, const void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    if (fseek(driver->img_file, offset * driver->buffer_size, SEEK_SET) != 0) {
        return -1;
    }
    return fwrite(buffer, 1, driver->buffer_size, driver->img_file);
}

/**
 * Read raw bytes at an absolute byte offset (used by the WL layer, which
 * needs addressing independent of the FAT sector size).
 * @param driver Pointer to IPDriver structure
 * @param offset Absolute byte offset in the image file
 * @param buffer Buffer to store read data
 * @param size Number of bytes to read
 * @return Number of bytes read if success, -1 if failed
 */
int ip_driver_read_bytes(IPDriver* driver, uint64_t offset, void* buffer, uint32_t size) {
    if (!driver || !driver->img_file || !buffer) return -1;

    if (fseek(driver->img_file, (long)offset, SEEK_SET) != 0) {
        return -1;
    }
    return (int)fread(buffer, 1, size, driver->img_file);
}

/**
 * Write raw bytes at an absolute byte offset (used by the WL layer).
 * @param driver Pointer to IPDriver structure
 * @param offset Absolute byte offset in the image file
 * @param buffer Buffer containing data to write
 * @param size Number of bytes to write
 * @return Number of bytes written if success, -1 if failed
 */
int ip_driver_write_bytes(IPDriver* driver, uint64_t offset, const void* buffer, uint32_t size) {
    if (!driver || !driver->img_file || !buffer) return -1;

    if (fseek(driver->img_file, (long)offset, SEEK_SET) != 0) {
        return -1;
    }
    return (int)fwrite(buffer, 1, size, driver->img_file);
}

/**
 * Close IP Driver
 * @param driver Pointer to IPDriver structure
 */
void ip_driver_close(IPDriver* driver) {
    if (driver && driver->img_file) {
        fclose(driver->img_file);
        driver->img_file = NULL;
    }
}

