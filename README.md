# FAT File System

Dự án triển khai hệ thống file FAT đơn giản.

## Cấu trúc dự án

```
src/
  ├── fat/     - Module quản lý FAT
  ├── disk/    - Module đọc/ghi ổ đĩa
  └── app/     - Giao diện người dùng
```

## Build và chạy

```bash
# Build dự án
make

# Chạy chương trình
./fat_fs <đường dẫn ổ đĩa>
```

## Tính năng

- Đọc/ghi file và thư mục
- Quản lý FAT table
- Giao diện dòng lệnh đơn giản
