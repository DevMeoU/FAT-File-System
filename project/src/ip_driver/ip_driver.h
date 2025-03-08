/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Module IP Driver cung cấp interface để tương tác với thiết bị
 *   mạng IP, cho phép gửi và nhận dữ liệu qua giao thức IP.
 *********************************************************************/
#ifndef __IP_DRIVER_H
#define __IP_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************************
 * Include Files
 *********************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Constants */
#define IP_MAX_PACKET_SIZE    1500
#define IP_MIN_PACKET_SIZE    20
#define IP_DEFAULT_TTL        64

/* Status codes */
#define IP_SUCCESS            0x00
#define IP_ERROR             -1
#define IP_TIMEOUT           -2
#define IP_INVALID_PARAM     -3

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* IP Address structure */
typedef struct {
    uint8_t bytes[4];    /* Địa chỉ IPv4 dạng byte array */
} ip_addr_t;

/* IP Configuration */
typedef struct {
    ip_addr_t ip_addr;     /* Địa chỉ IP */
    ip_addr_t netmask;     /* Netmask */
    ip_addr_t gateway;     /* Gateway */
    bool dhcp_enabled;     /* Bật/tắt DHCP */
} ip_config_t;

/* Packet structure */
typedef struct {
    uint8_t *data;         /* Con trỏ đến dữ liệu */
    uint16_t length;       /* Độ dài dữ liệu */
    ip_addr_t src_addr;    /* Địa chỉ nguồn */
    ip_addr_t dst_addr;    /* Địa chỉ đích */
    uint8_t protocol;      /* Giao thức */
    uint8_t ttl;          /* Time to live */
} ip_packet_t;

/*********************************************************************
 * Public Function Prototypes
 *********************************************************************/

/**
 * @brief Khởi tạo IP Driver
 *
 * @param config Cấu hình IP
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_driver_init(const ip_config_t *config);

/**
 * @brief Gửi gói tin IP
 *
 * @param packet Gói tin cần gửi
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_driver_send(const ip_packet_t *packet);

/**
 * @brief Nhận gói tin IP
 *
 * @param packet Con trỏ đến buffer lưu gói tin nhận được
 * @param timeout_ms Thời gian timeout tính bằng ms
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_driver_receive(ip_packet_t *packet, uint32_t timeout_ms);

/**
 * @brief Cập nhật cấu hình IP
 *
 * @param config Cấu hình IP mới
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
int32_t ip_driver_config(const ip_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* __IP_DRIVER_H */

/*********************************************************************
 * UUID: 7be660d6-55fa-417e-b30d-c44eecf89b71
 *********************************************************************/

