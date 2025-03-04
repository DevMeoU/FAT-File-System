/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/
#define _FILE_OFFSET_BITS 64 /* Enable 64-bit file offsets */

/*********************************************************************
 * Include
 *********************************************************************/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "ip_driver.h"

/*********************************************************************
 * Implementations
 *********************************************************************/
static FILE *file = NULL;

int ip_driver_init(const char *img_path) {
    file = fopen(img_path, "r+b");
    if (!file) {
        /** Lỗi mở file */
        return -1;
    }
    return 0;
}

int ip_driver_read(unsigned int offset, unsigned char *buffer, size_t size) {
    fseek(file, offset, SEEK_SET);
    size_t bytes_read = fread(buffer, 1, size, file);
    return (bytes_read == size) ? 0 : -1;
}

int ip_driver_write(unsigned int offset, const unsigned char *buffer, size_t size) {
    fseek(file, offset, SEEK_SET);
    size_t bytes_written = fwrite(buffer, 1, size, file);
    return (bytes_written == size) ? 0 : -1;
}

void ip_driver_close() {
    if (file) {
        fclose(file);
    }
}

