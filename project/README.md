# FAT File System Manager

## 1. Tổng Quan Hệ Thống

### 1.1. Kiến Trúc Tổng Thể
```
Application Layer (CLI)
      ↓
Middleware Layer
      ↓ 
FAT Driver Layer
      ↓
Hardware Layer
```

### 1.2. Cấu Trúc Thư Mục
```
project/
└── src/
    ├── application/    # Giao diện CLI và xử lý lệnh người dùng
    ├── middleware/     # Lớp trung gian xử lý logic nghiệp vụ
    ├── fat_driver/    # Xử lý hệ thống tệp FAT12/16/32
    ├── hal/           # Trừu tượng hóa phần cứng
    ├── ip_driver/     # Giao tiếp I/O cấp thấp
    └── utilities/     # Các tiện ích (log, status...)
```

## 2. Chi Tiết Các Module

### 2.1. Application Layer
- **Chức năng**: Cung cấp giao diện dòng lệnh (CLI) cho người dùng
- **Các lệnh hỗ trợ**:
  - `help`: Hiển thị trợ giúp
  - `ls`: Liệt kê thư mục
  - `cd`: Thay đổi thư mục
  - `mkdir`: Tạo thư mục mới
  - `rmdir`: Xóa thư mục
  - `cat`: Đọc nội dung file
  - `write`: Ghi nội dung vào file
  - `rm`: Xóa file
  - `cp`: Sao chép file
  - `mv`: Di chuyển file
  - `exit`: Thoát chương trình

### 2.2. Middleware Layer
- **Chức năng**: Xử lý logic nghiệp vụ và chuyển đổi lệnh
- **Các tính năng**:
  - Xử lý lệnh đồng bộ/bất đồng bộ
  - Đọc/ghi file thông qua FAT Driver
  - Gửi/nhận dữ liệu qua mạng
  - Quản lý buffer và cache

### 2.3. FAT Driver Layer
- **Chức năng**: Quản lý hệ thống tệp FAT
- **Tính năng chính**:
  - Hỗ trợ FAT12/16/32
  - Quản lý boot sector và FAT table
  - Cache sector để tối ưu hiệu năng
  - Quản lý cluster chain
  - Xử lý file và thư mục

### 2.4. Hardware Abstraction Layer (HAL)
- **Chức năng**: Trừu tượng hóa truy cập phần cứng
- **Tính năng**:
  - Đọc/ghi sector
  - Quản lý buffer
  - Xử lý lỗi I/O

### 2.5. IP Driver Layer
- **Chức năng**: Giao tiếp I/O cấp thấp
- **Tính năng**:
  - Gửi/nhận gói tin
  - Quản lý kết nối
  - Xử lý timeout

## 3. Luồng Xử Lý Chính

### 3.1. Khởi tạo hệ thống
```
app_init()
   ↓
mid_init()
   ↓
fat_init()
   ├── Kiểm tra boot sector
   ├── Xác định loại FAT
   ├── Tính toán tham số
   └── Khởi tạo cache
```

### 3.2. Xử lý lệnh người dùng
```
app_run()
   ├── Hiển thị prompt
   ├── Đọc lệnh
   └── app_process_command()
       └── Gọi handler tương ứng
```

### 3.3. Thao tác file
```
1. Đọc file:
fat_open() → fat_read() → fat_close()

2. Ghi file:
fat_open() → fat_write() → fat_close()
```

## 4. Cơ Chế Cache

### 4.1. Sector Cache
- Sử dụng LRU (Least Recently Used)
- Cache size: 16 sectors
- Write-back policy cho sector bẩn

### 4.2. FAT Cache
- Cache các entry FAT thường dùng
- Tối ưu cho cluster chain

## 5. Xử Lý Lỗi
- Kiểm tra tham số đầu vào
- Xử lý lỗi I/O
- Recovery từ sector bẩn
- Backup FAT table

## 6. Hướng Phát Triển
1. Hoàn thiện các hàm TODO:
   - `fat_seek()`
   - `fat_stat()`
   - `fat_unlink()`
   - `fat_mkdir()`
   - `fat_rmdir()`

2. Tối ưu hiệu năng:
   - Prefetching
   - Write buffering
   - Defragmentation

3. Tính năng nâng cao:
   - Hỗ trợ long filename
   - Journal ghi
   - Sao lưu tự động

## 7. Tác Giả
- Ducson9112k

## 8. UUID
- Application: 9b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
- Middleware: 8b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9
- FAT Driver: 4b8c3e2d-1a4f-4e85-9c6d-f8b2e3a1d5c9 