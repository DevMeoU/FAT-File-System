/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Interface cho IP Storage module, cung cấp các hàm để truy cập
 *   thiết bị lưu trữ qua giao thức IP. Module này đóng vai trò là
 *   một storage driver cụ thể trong hệ thống.
 *********************************************************************/
#ifndef __IP_STORAGE_H
#define __IP_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "../common/storage_interface.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Status codes */
#define IP_SUCCESS              0x00
#define IP_ERROR               -1
#define IP_TIMEOUT            -2
#define IP_INVALID_PARAM      -3
#define IP_NOT_READY          -4

/* Protocol constants */
#define IP_VERSION             4
#define IP_HEADER_LENGTH      20
#define IP_MIN_PACKET_SIZE    IP_HEADER_LENGTH
#define IP_MAX_PACKET_SIZE    1500
#define IP_DEFAULT_TTL        64

/* Storage constants */
#define IP_SECTOR_SIZE        512
#define IP_MAX_SECTORS        0xFFFFFFFF

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* IP Address */
typedef struct {
    uint8_t bytes[4];
} ip_addr_t;

/* IP Configuration */
typedef struct {
    ip_addr_t ip_addr;     /* Địa chỉ IP của thiết bị */
    ip_addr_t netmask;     /* Subnet mask */
    ip_addr_t gateway;     /* Default gateway */
    uint16_t port;         /* Port number */
    uint32_t timeout;      /* Timeout cho các thao tác (ms) */
} ip_config_t;

/* IP Packet */
typedef struct {
    ip_addr_t src_addr;    /* Địa chỉ nguồn */
    ip_addr_t dst_addr;    /* Địa chỉ đích */
    uint8_t protocol;      /* Giao thức */
    uint8_t ttl;          /* Time to live */
    uint8_t *data;        /* Con trỏ đến dữ liệu */
    uint16_t length;      /* Độ dài dữ liệu */
} ip_packet_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo IP Storage module
 * 
 * @param config Con trỏ đến cấu hình
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_storage_init(const ip_config_t *config);

/**
 * @brief Đọc một sector từ thiết bị
 * 
 * @param sector Số thứ tự sector
 * @param buffer Buffer lưu dữ liệu
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_read_sector(uint32_t sector, uint8_t *buffer);

/**
 * @brief Ghi một sector xuống thiết bị
 * 
 * @param sector Số thứ tự sector
 * @param buffer Buffer chứa dữ liệu
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_write_sector(uint32_t sector, const uint8_t *buffer);

/**
 * @brief Gửi một gói tin IP
 * 
 * @param packet Con trỏ đến gói tin
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_send_packet(const ip_packet_t *packet);

/**
 * @brief Nhận một gói tin IP
 * 
 * @param packet Con trỏ đến buffer lưu gói tin
 * @param timeout Thời gian chờ tối đa (ms)
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_receive_packet(ip_packet_t *packet, uint32_t timeout);

/**
 * @brief Cập nhật cấu hình IP
 * 
 * @param config Con trỏ đến cấu hình mới
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_update_config(const ip_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* __IP_STORAGE_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/

