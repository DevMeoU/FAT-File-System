#include "ip_driver.h"
#include "ip_driver_private.h"
#include <string.h>

int ip_driver_init(IPDriver* driver, const char* img_path) {
    if (!driver || !img_path) return -1;
    
    const char* ext = strrchr(img_path, '.');
    if (!ext || strcmp(ext, ".img") != 0) return -1;
    
    driver->img_file = fopen(img_path, "rb+");
    if (!driver->img_file) return -1;
    
    return 0;
}

int ip_driver_read_sector(IPDriver* driver, uint32_t offset, void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, offset * driver->buffer_size, SEEK_SET);
    return fread(buffer, 1, driver->buffer_size, driver->img_file);
}

int ip_driver_write_sector(IPDriver* driver, uint32_t offset, const void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, offset * driver->buffer_size, SEEK_SET);
    return fwrite(buffer, 1, driver->buffer_size, driver->img_file);
}

void ip_driver_close(IPDriver* driver) {
    if (driver && driver->img_file) {
        fclose(driver->img_file);
        driver->img_file = NULL;
    }
}
