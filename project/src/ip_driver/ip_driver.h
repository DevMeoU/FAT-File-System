/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#ifndef IP_DRIVER_H
#define IP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include
 *********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h> // Include for size_t

/*********************************************************************
 * Define
 *********************************************************************/

 /*********************************************************************
 * Function prototypes
 *********************************************************************/
/**
 * @brief Initialize the IP driver.
 *
 * @param img_path Path to the image file to be opened.
 * @return 0 if the file is successfully opened, -1 otherwise.
 */
int ip_driver_init(const char *img_path);

/**
 * @brief Reads data from the image file into the provided buffer.
 *
 * This function seeks to the specified offset in the file and reads
 * the specified number of bytes into the buffer.
 *
 * @param offset The offset in the file from which to start reading.
 * @param buffer The buffer where the read data will be stored.
 * @param size The number of bytes to read from the file.
 * @return 0 if the read operation is successful, -1 if an error occurs.
 */
int ip_driver_read(unsigned int offset, unsigned char *buffer, size_t size);

/**
 * @brief Writes data from the provided buffer into the image file.
 *
 * This function seeks to the specified offset in the file and writes
 * the specified number of bytes from the buffer.
 *
 * @param offset The offset in the file from which to start writing.
 * @param buffer The buffer containing the data to write.
 * @param size The number of bytes to write from the buffer.
 * @return 0 if the write operation is successful, -1 if an error occurs.
 */
int ip_driver_write(unsigned int offset, const unsigned char *buffer, size_t size);

/**
 * @brief Closes the image file.
 *  
 * This function simply checks if the file pointer is non-null and
 * closes the file using fclose if it is.
 * */
void ip_driver_close();

#ifdef __cplusplus
}
#endif

#endif /* IP_DRIVER_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/

