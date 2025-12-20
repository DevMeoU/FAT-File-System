# Triển khai Hệ thống Tệp FAT

[![Language](https://img.shields.io/badge/Language-C11-blue.svg)](https://en.wikipedia.org/wiki/C11_(C_standard_revision))
[![Build System](https://img.shields.io/badge/Build-Makefile-green.svg)](https://www.gnu.org/software/make/)
[![Platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-lightgrey.svg)]()
[![License](https://img.shields.io/badge/License-MIT-orange.svg)]()

[🇺🇸 **English Version / Phiên bản Tiếng Anh**](Readme.md)

---

## 📖 Tóm tắt

Dự án này là một bản triển khai **hệ thống tệp FAT12** bằng ngôn ngữ C ở tầng người dùng (userspace), được thiết kế để chạy trên Windows/Linux. Nó mô phỏng các tương tác phần cứng cấp thấp bằng cách coi một file ảnh đĩa thô (`floppy.img`) như một thiết bị vật lý.

Mục tiêu chính là minh họa **kiến trúc driver**, **mô hình thiết kế phân tầng**, và **thao tác hệ thống tệp thô** mà không phụ thuộc vào driver hệ thống tệp của hệ điều hành máy chủ.

---

## 📑 Mục lục
- [Kiến trúc](#-kiến-trúc)
  - [Thiết kế Phân tầng](#thiết-kế-phân-tầng)
  - [Luồng Dữ liệu](#luồng-dữ-liệu)
- [Cấu trúc Dự án](#-cấu-trúc-dự-án)
- [Chi tiết Thành phần](#-chi-tiết-thành-phần)
- [Biên dịch & Chạy](#-biên-dịch--chạy)
- [Hình ảnh](#-hình-ảnh)

---

## 🏗 Kiến trúc

### Thiết kế Phân tầng
Dự án tuân theo kiến trúc phân tầng nghiêm ngặt để đảm bảo tính module và tách biệt các mối quan tâm.

```mermaid
graph TD
    User([Người dùng]) <--> App[Tầng Ứng dụng]
    App <--> MW[Tầng Middleware]
    MW <--> FAT[Driver Hệ thống tệp FAT]
    FAT <--> HAL[Tầng Trừu tượng Phần cứng]
    HAL <--> IP[IP Driver (I/O Cấp thấp)]
    IP <--> IMG[(floppy.img)]
```

### Luồng Dữ liệu
1.  **Application**: Người dùng yêu cầu một file (ví dụ: `cat file.txt`).
2.  **Middleware**: Xác thực yêu cầu và điều phối hoạt động.
3.  **FAT Driver**: Phân tích bảng FAT và Thư mục gốc để tìm các cluster.
4.  **HAL**: Chuyển đổi các tương tác logic (sector) thành offset vật lý.
5.  **IP Driver**: Thực hiện `fseek`/`fread` thô trên file ảnh đĩa.

---

## 📂 Cấu trúc Dự án

Mã nguồn được tổ chức thành các module logic trong thư mục `src`:

```text
Project Root
├── 📁 build/              # Sản phẩm sau khi build
├── 📁 images/             # File ảnh đĩa (floppy.img)
├── 📁 src/
│   ├── 📁 application/    # CLI & Giao diện người dùng
│   ├── 📁 middleware/     # Logic nghiệp vụ & Xử lý dữ liệu
│   ├── 📁 fat_driver/     # Logic FAT12 (BootSector, FAT, RootDir)
│   ├── 📁 hal/            # Chuyển đổi Sector-sang-Offset
│   ├── 📁 ip_driver/      # File I/O thô (fseek, fread)
│   ├── 📁 common/         # Các kiểu dữ liệu chung (integers, error codes)
│   └── 📁 utilities/      # Cấu trúc dữ liệu (LinkedList, Log)
├── CMakeLists.txt         # Cấu hình build CMake
├── Makefile               # Cấu hình GNU Make
└── README.md              # File tài liệu này
```

---

## 🔧 Chi tiết Thành phần

<details>
<summary><b>1. IP Driver (I/O Cấp thấp)</b></summary>

Giao diện tới bộ nhớ "vật lý". Nó xử lý nghiêm ngặt các thao tác ở cấp độ byte.
*   **Vai trò**: Mở/Đóng `floppy.img`, Đọc/Ghi byte thô tại các offset.
*   **API chính**: `ip_driver_read`, `ip_driver_write`.
</details>

<details>
<summary><b>2. Hardware Abstraction Layer (HAL)</b></summary>

Dịch các khái niệm hệ thống tệp (sector) thành khái niệm lưu trữ (offset).
*   **Vai trò**: Định địa chỉ theo Sector (LBA).
*   **Hoạt động**: `Offset = Sector_Index * Sector_Size`.
</details>

<details>
<summary><b>3. FAT Driver</b></summary>

Logic cốt lõi của hệ thống tệp. Nó hiểu cấu trúc đĩa.
*   **Vai trò**: Phân tích Boot Sector, duyệt chuỗi FAT, và đọc các mục nhập thư mục.
*   **Tính năng**: Hỗ trợ tên file dài (LFN - giới hạn), liên kết Cluster.
</details>

<details>
<summary><b>4. Middleware & Application</b></summary>

*   **Middleware**: Cầu nối dữ liệu thô từ driver tới ứng dụng (ví dụ: chuyển buffer thành chuỗi, xử lý lỗi).
*   **Application**: Một CLI wrapper cho phép người dùng tương tác với hệ thống (các lệnh như `ls`, `cat`).
</details>

---

## 🚀 Biên dịch & Chạy

### Yêu cầu tiên quyết
*   **GCC** (MinGW cho Windows hoặc GCC gốc cho Linux)
*   **Make**

### Biên dịch

Bạn có thể build dự án bằng `Makefile` được cung cấp:

```bash
# Build bản release (Tối ưu hóa)
make release

# Build bản debug (Kèm biểu tượng debug)
make debug

# Xóa các file build cũ
make clean
```

### Thực thi

Chạy ứng dụng trỏ tới file ảnh đĩa của bạn:

```bash
# Chạy với cài đặt mặc định
make run
```

Hoặc chạy thủ công:
```bash
./build/bin/main.exe images/floppy.img
```

---

## 📸 Hình ảnh

| Liệt kê thư mục (`ls`) | Nội dung file (`cat`) |
|:-------------------------:|:--------------------:|
| ![Listing](image/README/1742297625445.png) | ![Reading](image/README/1742297655626.png) |

**Giao diện chi tiết:**
![Detailed View](image/README/1742297708543.png)

---

## 📜 Giấy phép
Dự án này là mã nguồn mở và được phát hành dưới các điều khoản của **Giấy phép MIT**.
