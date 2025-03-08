/*********************************************************************
 * ✨ Author: Ducson9112k 🌟
 * 
 * Description:
 *   Private header cho IP Storage module, định nghĩa các cấu trúc và
 *   hàm nội bộ chỉ sử dụng trong module. KHÔNG export các định nghĩa
 *   này ra bên ngoài module.
 *********************************************************************/
#ifndef __IP_STORAGE_PRIVATE_H
#define __IP_STORAGE_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ip_storage.h"

/*********************************************************************
 * Macro Definitions
 *********************************************************************/

/* Module states */
#define IP_STATE_UNINITIALIZED   0
#define IP_STATE_INITIALIZED     1
#define IP_STATE_ERROR          2

/* Debug configurations */
#define IP_DEBUG_ENABLED        1
#define IP_DEBUG_LEVEL          2

/* Protocol constants */
#define IP_PROTOCOL_STORAGE     0x1F
#define IP_STORAGE_PORT         5000
#define IP_STORAGE_TIMEOUT      1000

/* Buffer sizes */
#define IP_TX_BUFFER_SIZE       2048
#define IP_RX_BUFFER_SIZE       2048
#define IP_CACHE_SIZE          16

/* Performance optimizations */
#define IP_ENABLE_CACHE         1
#define IP_ENABLE_DMA          1
#define IP_MAX_RETRIES         3

/*********************************************************************
 * Type Definitions
 *********************************************************************/

/* IP Header */
typedef struct __attribute__((packed)) {
    uint8_t version_ihl;      /* Version (4 bits) + Header length (4 bits) */
    uint8_t tos;              /* Type of service */
    uint16_t total_length;    /* Total length */
    uint16_t id;              /* Identification */
    uint16_t flags_offset;    /* Flags (3 bits) + Fragment offset (13 bits) */
    uint8_t ttl;              /* Time to live */
    uint8_t protocol;         /* Protocol */
    uint16_t checksum;        /* Header checksum */
    uint8_t src_addr[4];      /* Source address */
    uint8_t dst_addr[4];      /* Destination address */
} ip_header_t;

/* Storage Command */
typedef struct __attribute__((packed)) {
    uint8_t cmd;              /* Command code */
    uint8_t flags;            /* Command flags */
    uint16_t sector_count;    /* Number of sectors */
    uint32_t start_sector;    /* Starting sector */
    uint32_t reserved;        /* Reserved for future use */
} ip_storage_cmd_t;

/* Cache Entry */
typedef struct {
    uint32_t sector;          /* Sector number */
    uint8_t data[IP_SECTOR_SIZE]; /* Sector data */
    bool valid;               /* Cache valid flag */
    bool dirty;               /* Cache modified flag */
    uint32_t access_count;    /* Access counter */
    uint32_t last_access;     /* Last access time */
} ip_cache_entry_t;

/* Module Context */
typedef struct {
    uint8_t state;            /* Module state */
    ip_config_t config;       /* Configuration */
    uint16_t packet_id;       /* Packet ID counter */
    uint32_t tx_count;        /* Transmit counter */
    uint32_t rx_count;        /* Receive counter */
    uint32_t error_count;     /* Error counter */
    
    /* Cache management */
    ip_cache_entry_t cache[IP_CACHE_SIZE];
    uint32_t cache_hits;
    uint32_t cache_misses;
    
    /* DMA management */
    void *dma_tx_buffer;
    void *dma_rx_buffer;
    bool dma_busy;
} ip_context_t;

/*********************************************************************
 * Private Function Prototypes
 *********************************************************************/

/**
 * @brief Tính checksum cho header IP
 * 
 * @param header Con trỏ đến header
 * @return Giá trị checksum
 */
static uint16_t ip_calculate_checksum(const ip_header_t *header);

/**
 * @brief Xử lý gói tin IP nhận được
 * 
 * @param header Con trỏ đến header
 * @param data Con trỏ đến dữ liệu
 * @param length Độ dài dữ liệu
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t ip_process_packet(const ip_header_t *header,
                               const uint8_t *data,
                               uint16_t length);

/**
 * @brief Kiểm tra địa chỉ IP có hợp lệ
 * 
 * @param addr Con trỏ đến địa chỉ IP
 * @return true nếu hợp lệ, false nếu không hợp lệ
 */
static bool ip_is_valid_address(const ip_addr_t *addr);

/**
 * @brief Quản lý cache
 * 
 * @param sector Sector number
 * @param data Con trỏ đến dữ liệu
 * @param write true nếu ghi, false nếu đọc
 * @return IP_SUCCESS nếu thành công, mã lỗi nếu thất bại
 */
static int32_t ip_cache_manage(uint32_t sector, uint8_t *data, bool write);

#ifdef __cplusplus
}
#endif

#endif /* __IP_STORAGE_PRIVATE_H */ 