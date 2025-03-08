/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Định nghĩa private cho IP Driver module.
 *   KHÔNG sử dụng trực tiếp các định nghĩa này từ bên ngoài module.
 *********************************************************************/
#ifndef __IP_DRIVER_PRIVATE_H
#define __IP_DRIVER_PRIVATE_H

#include "ip_driver.h"

/*********************************************************************
 * Private Macro Definitions
 *********************************************************************/

/* Module States */
#define IP_STATE_UNINITIALIZED  0U
#define IP_STATE_INITIALIZED    1U
#define IP_STATE_RUNNING       2U
#define IP_STATE_ERROR         3U

/* Debug Configuration */
#define IP_DEBUG_MODE          0U

/* Protocol Constants */
#define IP_VERSION            4U
#define IP_HEADER_LENGTH      20U
#define IP_MAX_TTL           255U

/* Buffer Sizes */
#define IP_RX_BUFFER_SIZE    2048U
#define IP_TX_BUFFER_SIZE    2048U

/* Timeouts */
#define IP_DEFAULT_TIMEOUT    1000U  /* ms */
#define IP_RETRY_TIMEOUT     100U    /* ms */
#define IP_MAX_RETRIES       3U

/*********************************************************************
 * Private Type Definitions
 *********************************************************************/

/* IP Header Structure */
typedef struct {
    uint8_t version_ihl;      /* Version (4 bits) + IHL (4 bits) */
    uint8_t tos;              /* Type of service */
    uint16_t total_length;    /* Total length */
    uint16_t id;              /* Identification */
    uint16_t flags_offset;    /* Flags (3 bits) + Fragment offset (13 bits) */
    uint8_t ttl;              /* Time to live */
    uint8_t protocol;         /* Protocol */
    uint16_t checksum;        /* Header checksum */
    uint32_t src_addr;        /* Source address */
    uint32_t dst_addr;        /* Destination address */
} ip_header_t;

/* IP Driver Context */
typedef struct {
    uint8_t state;                    /* Trạng thái driver */
    ip_config_t config;               /* Cấu hình IP */
    uint8_t rx_buffer[IP_RX_BUFFER_SIZE]; /* Buffer nhận */
    uint8_t tx_buffer[IP_TX_BUFFER_SIZE]; /* Buffer gửi */
    uint16_t packet_id;               /* ID gói tin */
    uint32_t rx_count;                /* Số gói tin đã nhận */
    uint32_t tx_count;                /* Số gói tin đã gửi */
    uint32_t error_count;             /* Số lỗi đã xảy ra */
} ip_context_t;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Tính checksum cho header IP
 *
 * @param header Con trỏ đến header IP
 * @return Giá trị checksum
 */
static uint16_t ip_calculate_checksum(const ip_header_t *header);

/**
 * @brief Xử lý gói tin nhận được
 *
 * @param header Con trỏ đến header IP
 * @param data Con trỏ đến dữ liệu
 * @param length Độ dài dữ liệu
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t ip_process_packet(const ip_header_t *header, 
                               const uint8_t *data, 
                               uint16_t length);

/**
 * @brief Kiểm tra tính hợp lệ của địa chỉ IP
 *
 * @param addr Địa chỉ IP cần kiểm tra
 * @return true nếu hợp lệ, false nếu không hợp lệ
 */
static bool ip_is_valid_address(const ip_addr_t *addr);

#endif /* __IP_DRIVER_PRIVATE_H */ 