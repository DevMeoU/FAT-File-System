
# FAT File System Explorer

## Tổng Quan Dự Án

**FAT File System Explorer** là một công cụ mạnh mẽ được phát triển bằng ngôn ngữ C, cho phép người dùng duyệt và tương tác với hệ thống tệp FAT từ các file ảnh đĩa. Dự án này kết hợp giao diện dòng lệnh thân thiện với kiến trúc phân tầng chuyên nghiệp, mang đến trải nghiệm tương tự Linux khi làm việc với các file hệ thống FAT.

## Đặc Điểm Nổi Bật

- **Giao Diện Terminal Trực Quan**: Hỗ trợ các lệnh Linux quen thuộc như `ls`, `cd`, `cat`, `help` và `evidence` để duyệt và tương tác với hệ thống tệp
- **Hỗ trợ Đa Dạng FAT**: Tương thích với FAT12, FAT16 và FAT32
- **Công Cụ Build Tích Hợp**: Shell tự động hóa với các tùy chọn build, run và clean
- **Đa Ngôn Ngữ**: Hỗ trợ cả tiếng Việt và tiếng Anh
- **Chế Độ Hoạt Động Linh Hoạt**: Hỗ trợ cả chế độ read-only và read-write

## Kiến Trúc Kỹ Thuật

Dự án được thiết kế theo mô hình phân tầng rõ ràng:

1. **IP Driver**: Tương tác trực tiếp với file ảnh đĩa
2. **HAL (Hardware Abstraction Layer)**: Trừu tượng hóa thao tác đọc/ghi sector
3. **FAT Driver**: Xử lý logic hệ thống tệp FAT
4. **Middleware**: Chuyển đổi dữ liệu thành thông tin người dùng
5. **Application**: Giao diện người dùng dạng shell

## Ứng Dụng Thực Tế

FAT File System Explorer lý tưởng cho:

- Phân tích và khôi phục dữ liệu từ các thiết bị lưu trữ cũ
- Học tập và nghiên cứu về cấu trúc hệ thống tệp
- Phát triển và kiểm thử ứng dụng làm việc với FAT
- Truy xuất dữ liệu từ các file ảnh đĩa mềm và các thiết bị lưu trữ khác

## Công Nghệ Sử Dụng

- **Ngôn Ngữ**: C thuần túy
- **Build System**: Makefile tùy chỉnh
- **Giao Diện**: Terminal-based với mã màu ANSI
- **Kiến Trúc**: Modular, phân tầng với khả năng mở rộng cao

---

Dự án này thể hiện khả năng thiết kế phần mềm hệ thống với kiến trúc rõ ràng, tập trung vào hiệu suất và trải nghiệm người dùng. FAT File System Explorer không chỉ là công cụ hữu ích cho việc làm việc với hệ thống tệp FAT mà còn là minh chứng cho kỹ năng lập trình hệ thống và thiết kế phần mềm.
