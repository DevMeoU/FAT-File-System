#!/bin/bash

# Danh sách các thư mục cần thiết
required_dirs=(
    "src"
    "src/ip_driver"
    "src/hal"
    "src/fat_driver"
    "src/middleware" 
    "src/application"
    "src/utilities"
    "src/utilities/log"
    "src/utilities/linkedlist"
    "src/common"
    "obj"
    "bin"
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
cat > src/ip_driver/ip_driver.h << 'EOF'
#ifndef IP_DRIVER_H
#define IP_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "../common/common_types.h"

typedef struct {
    FILE* img_file;
    uint32_t sector_size;
} IPDriver;

int ip_driver_init(IPDriver* driver, const char* img_path);
int ip_driver_read_sector(IPDriver* driver, uint32_t sector_number, void* buffer);
int ip_driver_write_sector(IPDriver* driver, uint32_t sector_number, const void* buffer);
void ip_driver_close(IPDriver* driver);

#endif
EOF

cat > src/ip_driver/ip_driver.c << 'EOF'
#include "ip_driver.h"
#include <string.h>

int ip_driver_init(IPDriver* driver, const char* img_path) {
    if (!driver || !img_path) return -1;
    
    const char* ext = strrchr(img_path, '.');
    if (!ext || strcmp(ext, ".img") != 0) return -1;
    
    driver->img_file = fopen(img_path, "rb+");
    if (!driver->img_file) return -1;
    
    driver->sector_size = 512;
    return 0;
}

int ip_driver_read_sector(IPDriver* driver, uint32_t sector_number, void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET);
    return fread(buffer, 1, driver->sector_size, driver->img_file);
}

int ip_driver_write_sector(IPDriver* driver, uint32_t sector_number, const void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET);
    return fwrite(buffer, 1, driver->sector_size, driver->img_file);
}

void ip_driver_close(IPDriver* driver) {
    if (driver && driver->img_file) {
        fclose(driver->img_file);
        driver->img_file = NULL;
    }
}
EOF

# Copy các file source từ thư mục gốc
echo "3. Copy source files từ thư mục gốc..."

# HAL
cp -r ../src/hal/* src/hal/

# FAT Driver
cp -r ../src/fat_driver/* src/fat_driver/

# Middleware
cp -r ../src/middleware/* src/middleware/

# Application
cp -r ../src/application/* src/application/

# Utilities
cp -r ../src/utilities/log/* src/utilities/log/
cp -r ../src/utilities/linkedlist/* src/utilities/linkedlist/

# Common
cp -r ../src/common/* src/common/

# Main
cp ../src/main.c src/

# Tạo Makefile
echo "4. Tạo Makefile..."
cat > Makefile << 'EOF'
# Compiler và flags
CC = gcc
CFLAGS = -Wall -Wextra -I./src
LDFLAGS = 

# Thư mục
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Danh sách các source files
SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(SRC_DIR)/ip_driver/*.c) \
       $(wildcard $(SRC_DIR)/hal/*.c) \
       $(wildcard $(SRC_DIR)/fat_driver/*.c) \
       $(wildcard $(SRC_DIR)/middleware/*.c) \
       $(wildcard $(SRC_DIR)/application/*.c) \
       $(wildcard $(SRC_DIR)/utilities/log/*.c) \
       $(wildcard $(SRC_DIR)/utilities/linkedlist/*.c)

# Chuyển .c thành .o
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Tên executable
TARGET = $(BIN_DIR)/fat_filesystem

# Phần build chính
all: directories $(TARGET)

# Tạo thư mục cần thiết
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/ip_driver
	@mkdir -p $(OBJ_DIR)/hal
	@mkdir -p $(OBJ_DIR)/fat_driver
	@mkdir -p $(OBJ_DIR)/middleware
	@mkdir -p $(OBJ_DIR)/application
	@mkdir -p $(OBJ_DIR)/utilities/log
	@mkdir -p $(OBJ_DIR)/utilities/linkedlist

# Link các object files thành executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile các source files thành object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild directories
EOF

# Tạo README
echo "5. Tạo README..."
cat > README.md << 'EOF'
# FAT File System Manager

## Cấu trúc dự án
```
</file>

<rewritten_file>
```
#!/bin/bash

# Danh sách các thư mục cần thiết
required_dirs=(
    "src"
    "src/ip_driver"
    "src/hal"
    "src/fat_driver"
    "src/middleware" 
    "src/application"
    "src/utilities"
    "src/utilities/log"
    "src/utilities/linkedlist"
    "src/common"
    "obj"
    "bin"
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
cat > src/ip_driver/ip_driver.h << 'EOF'
#ifndef IP_DRIVER_H
#define IP_DRIVER_H

#include <stdio.h>
#include <stdint.h>
#include "../common/common_types.h"

typedef struct {
    FILE* img_file;
    uint32_t sector_size;
} IPDriver;

int ip_driver_init(IPDriver* driver, const char* img_path);
int ip_driver_read_sector(IPDriver* driver, uint32_t sector_number, void* buffer);
int ip_driver_write_sector(IPDriver* driver, uint32_t sector_number, const void* buffer);
void ip_driver_close(IPDriver* driver);

#endif
EOF

cat > src/ip_driver/ip_driver.c << 'EOF'
#include "ip_driver.h"
#include <string.h>

int ip_driver_init(IPDriver* driver, const char* img_path) {
    if (!driver || !img_path) return -1;
    
    const char* ext = strrchr(img_path, '.');
    if (!ext || strcmp(ext, ".img") != 0) return -1;
    
    driver->img_file = fopen(img_path, "rb+");
    if (!driver->img_file) return -1;
    
    driver->sector_size = 512;
    return 0;
}

int ip_driver_read_sector(IPDriver* driver, uint32_t sector_number, void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET);
    return fread(buffer, 1, driver->sector_size, driver->img_file);
}

int ip_driver_write_sector(IPDriver* driver, uint32_t sector_number, const void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET);
    return fwrite(buffer, 1, driver->sector_size, driver->img_file);
}

void ip_driver_close(IPDriver* driver) {
    if (driver && driver->img_file) {
        fclose(driver->img_file);
        driver->img_file = NULL;
    }
}
EOF

# Copy các file source từ thư mục gốc
echo "3. Copy source files từ thư mục gốc..."

# HAL
cp -r ../src/hal/* src/hal/

# FAT Driver
cp -r ../src/fat_driver/* src/fat_driver/

# Middleware
cp -r ../src/middleware/* src/middleware/

# Application
cp -r ../src/application/* src/application/

# Utilities
cp -r ../src/utilities/log/* src/utilities/log/
cp -r ../src/utilities/linkedlist/* src/utilities/linkedlist/

# Common
cp -r ../src/common/* src/common/

# Main
cp ../src/main.c src/

# Tạo Makefile
echo "4. Tạo Makefile..."
cat > Makefile << 'EOF'
# Compiler và flags
CC = gcc
CFLAGS = -Wall -Wextra -I./src
LDFLAGS = 

# Thư mục
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Danh sách các source files
SRCS = $(wildcard $(SRC_DIR)/*.c) \
       $(wildcard $(SRC_DIR)/ip_driver/*.c) \
       $(wildcard $(SRC_DIR)/hal/*.c) \
       $(wildcard $(SRC_DIR)/fat_driver/*.c) \
       $(wildcard $(SRC_DIR)/middleware/*.c) \
       $(wildcard $(SRC_DIR)/application/*.c) \
       $(wildcard $(SRC_DIR)/utilities/log/*.c) \
       $(wildcard $(SRC_DIR)/utilities/linkedlist/*.c)

# Chuyển .c thành .o
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Tên executable
TARGET = $(BIN_DIR)/fat_filesystem

# Phần build chính
all: directories $(TARGET)

# Tạo thư mục cần thiết
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(OBJ_DIR)/ip_driver
	@mkdir -p $(OBJ_DIR)/hal
	@mkdir -p $(OBJ_DIR)/fat_driver
	@mkdir -p $(OBJ_DIR)/middleware
	@mkdir -p $(OBJ_DIR)/application
	@mkdir -p $(OBJ_DIR)/utilities/log
	@mkdir -p $(OBJ_DIR)/utilities/linkedlist

# Link các object files thành executable
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile các source files thành object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild directories
EOF

# Tạo README
echo "5. Tạo README..."
cat > README.md << 'EOF'
# FAT File System Manager

## Cấu trúc dự án