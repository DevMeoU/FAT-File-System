# IP Driver Module

## Mô tả
Module IP Driver cung cấp interface để tương tác với thiết bị mạng IP, cho phép gửi và nhận dữ liệu qua giao thức IP. Module này xử lý việc đóng gói IP, tính toán checksum, và quản lý các kết nối mạng cơ bản.

## Cấu trúc thư mục
```
ip_driver/
├── ip_driver.h         # Public API header
├── ip_driver_private.h # Private definitions
├── ip_driver.c        # Implementation
└── README.md         # Documentation
```

## API Public
Liệt kê và mô tả các API public của module:

### ip_driver_init
```c
int32_t ip_driver_init(const ip_config_t *config);
```
- Mô tả: Khởi tạo IP Driver với cấu hình cho trước
- Tham số:
  - config: Cấu hình IP bao gồm địa chỉ IP, netmask, gateway
- Trả về: IP_SUCCESS nếu thành công, mã lỗi nếu thất bại

### ip_driver_send
```c
int32_t ip_driver_send(const ip_packet_t *packet);
```
- Mô tả: Gửi gói tin IP
- Tham số:
  - packet: Gói tin cần gửi bao gồm dữ liệu và thông tin header
- Trả về: IP_SUCCESS nếu thành công, mã lỗi nếu thất bại

### ip_driver_receive
```c
int32_t ip_driver_receive(ip_packet_t *packet, uint32_t timeout_ms);
```
- Mô tả: Nhận gói tin IP
- Tham số:
  - packet: Buffer lưu gói tin nhận được
  - timeout_ms: Thời gian timeout tính bằng ms
- Trả về: IP_SUCCESS nếu thành công, mã lỗi nếu thất bại

### ip_driver_config
```c
int32_t ip_driver_config(const ip_config_t *config);
```
- Mô tả: Cập nhật cấu hình IP
- Tham số:
  - config: Cấu hình IP mới
- Trả về: IP_SUCCESS nếu thành công, mã lỗi nếu thất bại

## Kiểu dữ liệu
Mô tả các kiểu dữ liệu quan trọng:

### ip_addr_t
```c
typedef struct {
    uint8_t bytes[4];    /* Địa chỉ IPv4 dạng byte array */
} ip_addr_t;
```

### ip_config_t
```c
typedef struct {
    ip_addr_t ip_addr;     /* Địa chỉ IP */
    ip_addr_t netmask;     /* Netmask */
    ip_addr_t gateway;     /* Gateway */
    bool dhcp_enabled;     /* Bật/tắt DHCP */
} ip_config_t;
```

### ip_packet_t
```c
typedef struct {
    uint8_t *data;         /* Con trỏ đến dữ liệu */
    uint16_t length;       /* Độ dài dữ liệu */
    ip_addr_t src_addr;    /* Địa chỉ nguồn */
    ip_addr_t dst_addr;    /* Địa chỉ đích */
    uint8_t protocol;      /* Giao thức */
    uint8_t ttl;          /* Time to live */
} ip_packet_t;
```

## Cách sử dụng
Ví dụ về cách sử dụng IP Driver module:

```c
#include "ip_driver.h"

void example(void) {
    // Khởi tạo cấu hình
    ip_config_t config = {
        .ip_addr = {{192, 168, 1, 100}},
        .netmask = {{255, 255, 255, 0}},
        .gateway = {{192, 168, 1, 1}},
        .dhcp_enabled = false
    };

    // Khởi tạo driver
    if (ip_driver_init(&config) != IP_SUCCESS) {
        // Xử lý lỗi
        return;
    }

    // Chuẩn bị gói tin
    uint8_t data[] = "Hello, IP!";
    ip_packet_t packet = {
        .data = data,
        .length = sizeof(data),
        .src_addr = config.ip_addr,
        .dst_addr = {{192, 168, 1, 200}},
        .protocol = 1,  // ICMP
        .ttl = 64
    };

    // Gửi gói tin
    if (ip_driver_send(&packet) != IP_SUCCESS) {
        // Xử lý lỗi
        return;
    }

    // Nhận gói tin
    ip_packet_t rx_packet = {0};
    if (ip_driver_receive(&rx_packet, 1000) == IP_SUCCESS) {
        // Xử lý gói tin nhận được
    }
}
```

## Lưu ý
- Đảm bảo gọi ip_driver_init() trước khi sử dụng các API khác
- Kiểm tra kỹ các tham số đầu vào để tránh lỗi
- Xử lý timeout khi nhận gói tin
- Đảm bảo buffer đủ lớn khi nhận gói tin
- Cẩn thận với các địa chỉ IP không hợp lệ

## Tác giả
- Ducson9112k 