/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 *********************************************************************/

/*********************************************************************
 * Include
 *********************************************************************/
#include "ip_driver.h"
#include "hal.h"

/*********************************************************************
 * Define
 *********************************************************************/


/*********************************************************************
 * Static variables for configuration
 *********************************************************************/
static size_t g_sector_size = DEFAULT_SECTOR_SIZE;
static unsigned int g_max_sectors = 0; // 0 nghĩa là không giới hạn

/*********************************************************************
 * Implementations
 *********************************************************************/
int hal_init(const char *img_path) {
    return ip_driver_init(img_path);
}

int hal_read_sector(unsigned int sector, unsigned char *buffer) {
    /* Nếu max_sectors được cấu hình và sector yêu cầu vượt quá giới hạn, trả về lỗi */
    if (g_max_sectors > 0 && sector >= g_max_sectors) {
        return -1; // sector out of range
    }
    
    unsigned int offset = sector * g_sector_size;
    int result = ip_driver_read(offset, buffer, g_sector_size);
    return result == 0 ? 0 : -1;  /* Trả về 0 khi thành công, -1 khi có lỗi */
}

int hal_write_sector(unsigned int sector, const unsigned char *buffer) {
    /* Kiểm tra giới hạn sector nếu được cấu hình */
    if (g_max_sectors > 0 && sector >= g_max_sectors) {
        return -1; // sector out of range
    }
    
    unsigned int offset = sector * g_sector_size;
    int result = ip_driver_write(offset, buffer, g_sector_size);
    return result == 0 ? 0 : -1;  /* Trả về 0 khi thành công, -1 khi có lỗi */
}

/*
 * Hàm hal_configure cho phép cấu hình:
 *  - sector_size: kích thước của mỗi sector (ví dụ 512 byte)
 *  - max_sectors: số sector tối đa (nếu 0 thì không giới hạn)
 */
void hal_configure(size_t sector_size, unsigned int max_sectors) {
    g_sector_size = sector_size;
    g_max_sectors = max_sectors;
}
