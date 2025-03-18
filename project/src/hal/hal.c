#include "hal.h"
#include <string.h>

int hal_init(HAL* hal, const char* img_path, SectorSize sector_size) {
    if (!hal || !img_path) return -1;
    
    // Khởi tạo IP Driver với kích thước sector được chỉ định
    int result = ip_driver_init(&hal->ip_driver, img_path);
    if (result != 0) return -1;
    
    // Kiểm tra sector_size hợp lệ (512, 1024, 2048, 4096)
    if (sector_size != SECTOR_SIZE_512 && 
        sector_size != SECTOR_SIZE_1024 && 
        sector_size != SECTOR_SIZE_2048 && 
        sector_size != SECTOR_SIZE_4096) {
        ip_driver_close(&(hal->ip_driver));
        hal->ip_driver.img_file = NULL;
        return -1;
    }
    
    hal->ip_driver.buffer_size = sector_size;
    hal->sector_size = sector_size;
    return 0;
}

int hal_deinit(HAL* hal) {
    if (!hal) return -1;
    // Đóng IP Driver
    ip_driver_close(&hal->ip_driver);
    return 0;
}

int hal_read_sector(HAL* hal, uint32_t sector_number, void* buffer) {
    if (!hal || !buffer) return -1;
    
    return ip_driver_read_sector(&hal->ip_driver, sector_number, buffer);
}

int hal_write_sector(HAL* hal, uint32_t sector_number, const void* buffer) {
    if (!hal || !buffer) return -1;
    
    return ip_driver_write_sector(&hal->ip_driver, sector_number, buffer);
}

void hal_close(HAL* hal) {
    if (hal) {
        ip_driver_close(&hal->ip_driver);
    }
}

uint32_t hal_get_sector_size(HAL* hal) {
    if (!hal) return 0;
    
    return (uint32_t)hal->sector_size;
}
