#include "ip_driver.h"
#include "ip_driver_private.h"
#include <string.h>

int ip_driver_init(IPDriver* driver, const char* img_path, uint32_t sector_size) {
    if (!driver || !img_path) return -1;
    
    const char* ext = strrchr(img_path, '.');
    if (!ext || strcmp(ext, ".img") != 0) return -1;
    
    driver->img_file = fopen(img_path, "rb+");
    if (!driver->img_file) return -1;
    
    // Kiểm tra sector_size hợp lệ (512, 1024, 2048, 4096)
    if (sector_size != SECTOR_SIZE_512 && 
        sector_size != SECTOR_SIZE_1024 && 
        sector_size != SECTOR_SIZE_2048 && 
        sector_size != SECTOR_SIZE_4096) {
        fclose(driver->img_file);
        driver->img_file = NULL;
        return -1;
    }
    
    driver->sector_size = sector_size;
    return 0;
}

int ip_driver_read_sector(IPDriver* driver, uint32_t sector_number, void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET);
    return fread(buffer, 1, driver->sector_size, driver->img_file);
}

int ip_driver_write_sector(IPDriver* driver, uint32_t sector_number, const void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET);
    return fwrite(buffer, 1, driver->sector_size, driver->img_file);
}

void ip_driver_close(IPDriver* driver) {
    if (driver && driver->img_file) {
        fclose(driver->img_file);
        driver->img_file = NULL;
    }
}
