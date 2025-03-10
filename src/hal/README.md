# HAL Module

## Mô tả
Module HAL (Hardware Abstraction Layer) cung cấp interface để tương tác với phần cứng, cho phép truy cập các tài nguyên phần cứng một cách độc lập với nền tảng cụ thể. Module này đảm bảo tính di động của phần mềm giữa các nền tảng khác nhau.

## Cấu trúc thư mục
```
hal/
├── hal.h         # Public API header
├── hal_private.h # Private definitions
├── hal.c        # Implementation
└── README.md    # Documentation
```

## API Public
Liệt kê và mô tả các API public của module:

### hal_init
```c
int32_t hal_init(const hal_config_t *config);
```
- Mô tả: Khởi tạo HAL với cấu hình cho trước
- Tham số:
  - config: Cấu hình HAL bao gồm tốc độ clock, kích thước buffer...
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

### hal_read
```c
int32_t hal_read(void *buffer, uint32_t size, uint32_t timeout);
```
- Mô tả: Đọc dữ liệu từ thiết bị
- Tham số:
  - buffer: Buffer lưu dữ liệu
  - size: Số byte cần đọc
  - timeout: Thời gian timeout (ms)
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

### hal_write
```c
int32_t hal_write(const void *buffer, uint32_t size, uint32_t timeout);
```
- Mô tả: Ghi dữ liệu vào thiết bị
- Tham số:
  - buffer: Buffer chứa dữ liệu
  - size: Số byte cần ghi
  - timeout: Thời gian timeout (ms)
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

### hal_set_transfer_mode
```c
int32_t hal_set_transfer_mode(hal_transfer_mode_t mode);
```
- Mô tả: Cấu hình chế độ truyền
- Tham số:
  - mode: Chế độ truyền (polling/interrupt/DMA)
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

### hal_get_device_info
```c
int32_t hal_get_device_info(hal_device_info_t *info);
```
- Mô tả: Lấy thông tin thiết bị
- Tham số:
  - info: Con trỏ đến struct lưu thông tin
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

### hal_get_status
```c
int32_t hal_get_status(hal_status_t *status);
```
- Mô tả: Lấy trạng thái thiết bị
- Tham số:
  - status: Con trỏ đến struct lưu trạng thái
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

### hal_reset
```c
int32_t hal_reset(void);
```
- Mô tả: Reset thiết bị
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

### hal_register_callback
```c
int32_t hal_register_callback(void (*callback)(void *), uint32_t event_id);
```
- Mô tả: Đăng ký callback cho sự kiện
- Tham số:
  - callback: Hàm callback
  - event_id: ID sự kiện
- Trả về: HAL_SUCCESS nếu thành công, mã lỗi nếu thất bại

## Kiểu dữ liệu
Mô tả các kiểu dữ liệu quan trọng:

### hal_config_t
```c
typedef struct {
    uint32_t clock_speed;     /* Tốc độ clock (Hz) */
    uint32_t buffer_size;     /* Kích thước buffer */
    uint32_t timeout;         /* Timeout mặc định (ms) */
    bool interrupt_enable;    /* Cho phép ngắt */
} hal_config_t;
```

### hal_device_info_t
```c
typedef struct {
    uint32_t device_id;       /* ID thiết bị */
    uint32_t manufacturer_id; /* ID nhà sản xuất */
    uint32_t version;        /* Phiên bản */
    uint32_t capabilities;   /* Khả năng */
} hal_device_info_t;
```

### hal_transfer_mode_t
```c
typedef enum {
    HAL_MODE_POLLING = 0,    /* Chế độ polling */
    HAL_MODE_INTERRUPT,      /* Chế độ ngắt */
    HAL_MODE_DMA            /* Chế độ DMA */
} hal_transfer_mode_t;
```

## Cách sử dụng
Ví dụ về cách sử dụng HAL module:

```c
#include "hal.h"

void example(void) {
    // Khởi tạo cấu hình
    hal_config_t config = {
        .clock_speed = 48000000,    // 48MHz
        .buffer_size = 1024,        // 1KB
        .timeout = 1000,            // 1s
        .interrupt_enable = true
    };

    // Khởi tạo HAL
    if (hal_init(&config) != HAL_SUCCESS) {
        // Xử lý lỗi
        return;
    }

    // Cấu hình chế độ truyền
    hal_set_transfer_mode(HAL_MODE_INTERRUPT);

    // Đọc dữ liệu
    uint8_t buffer[64];
    if (hal_read(buffer, sizeof(buffer), 1000) != HAL_SUCCESS) {
        // Xử lý lỗi
        return;
    }

    // Xử lý dữ liệu...

    // Ghi dữ liệu
    uint8_t data[] = "Hello, HAL!";
    if (hal_write(data, sizeof(data), 1000) != HAL_SUCCESS) {
        // Xử lý lỗi
        return;
    }
}
```

## Lưu ý
- Đảm bảo gọi hal_init() trước khi sử dụng các API khác
- Kiểm tra kỹ các tham số đầu vào để tránh lỗi
- Xử lý timeout một cách phù hợp
- Chọn chế độ truyền phù hợp với yêu cầu
- Đăng ký callback nếu cần xử lý sự kiện
- Kiểm tra trạng thái thiết bị thường xuyên

## Tác giả
- Ducson9112k 