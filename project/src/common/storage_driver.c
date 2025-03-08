/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Triển khai các hàm của Storage Driver, sử dụng IP Driver làm
 *   backend mặc định.
 *********************************************************************/

#include <string.h>
#include "storage_driver.h"
#include "../ip_driver/ip_driver.h"

int32_t storage_read_sector(uint32_t sector, uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID_PARAMETER;
    }

    // Sử dụng IP driver làm backend mặc định
    int32_t ret = ip_read_sector(sector, buffer);
    if (ret != IP_SUCCESS) {
        return STATUS_READ_FAILED;
    }

    return STATUS_SUCCESS;
}

int32_t storage_write_sector(uint32_t sector, const uint8_t *buffer)
{
    if (!buffer) {
        return STATUS_INVALID_PARAMETER;
    }

    // Sử dụng IP driver làm backend mặc định
    int32_t ret = ip_write_sector(sector, buffer);
    if (ret != IP_SUCCESS) {
        return STATUS_WRITE_FAILED;
    }

    return STATUS_SUCCESS;
} 