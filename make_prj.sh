#!/bin/bash
# Danh sách các thư mục cần thiết theo cấu trúc mong muốn
required_dirs=(
    "project"
    "test"  # Thư mục test ngang hàng với project
    "test/Makefile"
    "project/src"
    "project/src/ip_driver"
    "project/src/hal"
    "project/src/fat_driver"
    "project/src/middleware"
    "project/src/application"
    "project/src/utilities"
    "project/src/utilities/linkedlist"
)

echo "Kiểm tra và tạo các thư mục cần thiết..."
# Tạo các thư mục nếu chưa tồn tại
for dir in "${required_dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        echo "Tạo thư mục: $dir"
        mkdir -p "$dir"
    else
        echo "Thư mục đã tồn tại: $dir"
    fi
done

# Xóa các thư mục thừa trong project/src (chỉ giữ: ip_driver, hal, fat_driver, middleware, application, utilities)
echo "Kiểm tra project/src để tìm thư mục thừa..."
if [ -d "project/src" ]; then
    cd project/src || exit 1
    for d in */ ; do
        d=${d%/}  # Xóa dấu gạch chéo cuối
        if [[ "$d" != "ip_driver" && "$d" != "hal" && "$d" != "fat_driver" && "$d" != "middleware" && "$d" != "application" && "$d" != "utilities" ]]; then
            echo "Xóa thư mục thừa trong src: $d"
            rm -rf "$d"
        fi
    done
    cd - > /dev/null
fi

# Xóa các thư mục thừa trong project/src/utilities (chỉ giữ: linkedlist)
echo "Kiểm tra project/src/utilities để tìm thư mục thừa..."
if [ -d "project/src/utilities" ]; then
    cd project/src/utilities || exit 1
    for d in */ ; do
        d=${d%/}
        if [[ "$d" != "linkedlist" ]]; then
            echo "Xóa thư mục thừa trong utilities: $d"
            rm -rf "$d"
        fi
    done
    cd - > /dev/null
fi

# Tạo các file mẫu nếu chưa tồn tại
echo "Tạo các file mẫu nếu chưa tồn tại..."
touch project/src/ip_driver/ip_driver.c project/src/ip_driver/ip_driver.h
touch project/src/hal/hal.c project/src/hal/hal.h
touch project/src/fat_driver/fat_driver.c project/src/fat_driver/fat_driver.h
touch project/src/middleware/middleware.c project/src/middleware/middleware.h
touch project/src/application/application.c project/src/application/application.h
touch project/src/utilities/linkedlist/linkedlist.c project/src/utilities/linkedlist/linkedlist.h
touch project/Makefile
touch test/test.c test/test.h  # Tạo thêm file .c và .h trong test
touch test/Makefile.mak
echo "Cấu trúc dự án đã được cập nhật theo yêu cầu."