/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Implementation của các hàm private trong IP Driver module
 *********************************************************************/

#include "ip_driver.h"
#include "ip_driver_private.h"

int32_t ip_read_sector(uint32_t sector, uint8_t *buffer) {
    // Kiểm tra tham số đầu vào
    if (!buffer) {
        return IP_INVALID_PARAM;
    }

    // Tránh cảnh báo tham số không sử dụng
    (void)sector;

    // TODO: Implement actual sector reading
    // Tạm thời trả về thành công
    return IP_SUCCESS;
}

int32_t ip_write_sector(uint32_t sector, const uint8_t *buffer) {
    // Kiểm tra tham số đầu vào
    if (!buffer) {
        return IP_INVALID_PARAM;
    }

    // Tránh cảnh báo tham số không sử dụng
    (void)sector;

    // TODO: Implement actual sector writing
    // Tạm thời trả về thành công
    return IP_SUCCESS;
}

/* Private function implementations */
static uint16_t ip_calculate_checksum(const ip_header_t *header) {
    uint32_t sum = 0;
    const uint16_t *data = (const uint16_t *)header;

    /* Calculate sum of 16-bit words */
    for (uint32_t i = 0; i < IP_HEADER_LENGTH/2; i++) {
        sum += data[i];
    }

    /* Add carry */
    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (uint16_t)~sum;
}

static int32_t ip_process_packet(const ip_header_t *header,
                               const uint8_t *data,
                               uint16_t length) {
    // Kiểm tra tham số đầu vào
    if (!header || !data || length < IP_MIN_PACKET_SIZE) {
        return IP_INVALID_PARAM;
    }

    // Kiểm tra version
    if ((header->version_ihl >> 4) != IP_VERSION) {
        return IP_ERROR;
    }

    // Kiểm tra header length
    if ((header->version_ihl & 0x0F) != IP_HEADER_LENGTH/4) {
        return IP_ERROR;
    }

    // Kiểm tra total length
    uint16_t total_length = (header->total_length >> 8) | 
                           (header->total_length << 8);
    if (total_length != length) {
        return IP_ERROR;
    }

    // Kiểm tra checksum
    uint16_t checksum = ip_calculate_checksum(header);
    if (checksum != 0) {
        return IP_ERROR;
    }

    return IP_SUCCESS;
}

static bool ip_is_valid_address(const ip_addr_t *addr) {
    if (!addr) {
        return false;
    }

    // Kiểm tra địa chỉ không hợp lệ
    if (addr->bytes[0] == 0 && addr->bytes[1] == 0 &&
        addr->bytes[2] == 0 && addr->bytes[3] == 0) {
        return false;
    }

    // Kiểm tra địa chỉ broadcast
    if (addr->bytes[0] == 255 && addr->bytes[1] == 255 &&
        addr->bytes[2] == 255 && addr->bytes[3] == 255) {
        return false;
    }

    return true;
} 