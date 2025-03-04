/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/

/*********************************************************************
 * Include
 *********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ip_driver.h"

/*********************************************************************
 * Define
 *********************************************************************/
#define _FILE_OFFSET_BITS 64 /* Enable 64-bit file offsets */

/*********************************************************************
 * Function prototypes
 *********************************************************************/

/*********************************************************************
 * Implementations
 *********************************************************************/
static FILE *file = NULL;

/**
 * @brief Initializes the IP driver by opening the specified image file.
 *
 * This function attempts to open the file located at the provided path
 * in read/write binary mode. If the file is successfully opened, a global
 * file pointer is set for subsequent read/write operations.
 *
 * @param img_path Path to the image file to be opened.
 * @return 0 if the file is successfully opened, -1 if an error occurs.
 */

int ip_driver_init(const char *img_path) {
    file = fopen(img_path, "r+b");
    if (!file) {
        return -1;
    }
    return 0;
}

/**
 * @brief Reads data from the image file into the provided buffer.
 *
 * This function seeks to the specified offset in the file and reads
 * the specified number of bytes into the buffer. It ensures that the
 * read operation starts from the correct position by using fseek.
 *
 * @param offset The offset in the file from which to start reading.
 * @param buffer The buffer where the read data will be stored.
 * @param size The number of bytes to read from the file.
 * @return 0 if the read operation is successful and the exact number
 *         of bytes is read, -1 if an error occurs or if fewer bytes
 *         are read.
 */

/**
 * @brief Reads data from the image file into the provided buffer.
 *
 * This function seeks to the specified offset in the file and reads
 * the specified number of bytes into the buffer. It ensures that the
 * read operation starts from the correct position by using fseek.
 *
 * @param offset The offset in the file from which to start reading.
 * @param buffer The buffer where the read data will be stored.
 * @param size The number of bytes to read from the file.
 * @return 0 if the read operation is successful and the exact number
 *         of bytes is read, -1 if an error occurs or if fewer bytes
 *         are read.
 */
int ip_driver_read(unsigned int offset, unsigned char *buffer, size_t size) {
    fseek(file, offset, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, size, file);
    return (bytes_read == size) ? 0 : -1;
}

/**
 * @brief Writes data from the provided buffer into the image file.
 *
 * This function seeks to the specified offset in the file and writes
 * the specified number of bytes from the buffer. It ensures that the
 * write operation starts from the correct position by using fseek.
 *
 * @param offset The offset in the file from which to start writing.
 * @param buffer The buffer containing the data to write.
 * @param size The number of bytes to write from the buffer.
 * @return 0 if the write operation is successful and the exact number
 *         of bytes is written, -1 if an error occurs or if fewer bytes
 *         are written.
 */
int ip_driver_write(unsigned int offset, const unsigned char *buffer, size_t size) {
    fseek(file, offset, SEEK_SET);
    size_t bytes_written = fwrite(buffer, 1, size, file);
    return (bytes_written == size) ? 0 : -1;
}

/**
 * @brief Closes the image file and releases any associated resources.
 *
 * This function simply checks if the file pointer is non-null and
 * closes the file using fclose if it is.
 */
void ip_driver_close() {
    if (file) {
        fclose(file);
    }
}

