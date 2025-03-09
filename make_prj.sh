#!/bin/bash

# Danh sách các thư mục cần thiết
required_dirs=(
    "project"
    "project/src"
    "project/src/ip_driver"
    "project/src/hal"
    "project/src/fat_driver"
    "project/src/middleware"
    "project/src/application"
    "project/src/utilities"
    "project/src/utilities/log"
    "project/src/utilities/linkedlist"
    "project/src/common"
)

echo "=== Tạo cấu trúc dự án FAT File System Manager ==="

# Tạo các thư mục
echo "1. Tạo cấu trúc thư mục..."
for dir in "${required_dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "  + Tạo: $dir"
        mkdir -p "$dir"
    else
        echo "  * Đã tồn tại: $dir"
    fi
done

# Tạo các file trong ip_driver
echo "2. Tạo files cho IP Driver..."
touch project/src/ip_driver/ip_driver.h
touch project/src/ip_driver/ip_driver.c
touch project/src/ip_driver/ip_driver_private.h
touch project/src/ip_driver/ip_driver_private.c

# Tạo các file trong hal
echo "3. Tạo files cho HAL..."
touch project/src/hal/hal.h
touch project/src/hal/hal.c
touch project/src/hal/hal_private.h

# Tạo các file trong fat_driver
echo "4. Tạo files cho FAT Driver..."
touch project/src/fat_driver/fat_driver.h
touch project/src/fat_driver/fat_driver.c
touch project/src/fat_driver/fat_driver_private.h
touch project/src/fat_driver/fat_driver_types.h
touch project/src/fat_driver/README.md

# Tạo các file trong middleware
echo "5. Tạo files cho Middleware..."
touch project/src/middleware/middleware.h
touch project/src/middleware/middleware.c

# Tạo các file trong application
echo "6. Tạo files cho Application..."
touch project/src/application/application.h
touch project/src/application/application.c

# Tạo các file trong utilities
echo "7. Tạo files cho Utilities..."
# Log module
touch project/src/utilities/log/print_color.h
touch project/src/utilities/log/print_color.c
# Linkedlist module
touch project/src/utilities/linkedlist/linkedlist.h
touch project/src/utilities/linkedlist/linkedlist.c

# Tạo các file trong common
echo "8. Tạo files cho Common..."
touch project/src/common/common_types.h

# Tạo Makefile và README
echo "9. Tạo Makefile và README..."
touch project/Makefile
touch project/README.md

echo "=== Hoàn thành tạo cấu trúc dự án ==="
echo "Cấu trúc thư mục:"
tree project/

echo "
Lưu ý:
1. Các file header (.h) và source (.c) đã được tạo
2. Cần thêm nội dung cho các file
3. Cập nhật Makefile để build dự án
4. Xem README.md để biết thêm chi tiết"
