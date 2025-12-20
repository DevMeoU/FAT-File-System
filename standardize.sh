#!/bin/bash

# Color codes
RED='\033[0;31m'
ORANGE='\033[0;33m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# Detect make command
if command -v mingw32-make >/dev/null 2>&1; then
    MAKE_CMD="mingw32-make"
else
    MAKE_CMD="make"
fi

echo -e "${CYAN}=== FAT File System Manager Standardization ===${NC}"

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

# Tạo các thư mục
echo -e "${YELLOW}1. Creating directory structure...${NC}"
for dir in "${required_dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        mkdir -p "$dir"
        echo -e "${GREEN}  + Created: $dir${NC}"
    else
        echo "  * Already exists: $dir"
    fi
done

# Tạo các file trong ip_driver
echo -e "${YELLOW}2. Creating IP Driver files...${NC}"
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
int ip_driver_read_buffer(IPDriver* driver, uint32_t sector_number, void* buffer);
int ip_driver_write_buffer(IPDriver* driver, uint32_t sector_number, const void* buffer);
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

int ip_driver_read_buffer(IPDriver* driver, uint32_t sector_number, void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    if (fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET) != 0) return -1;
    return fread(buffer, 1, driver->sector_size, driver->img_file);
}

int ip_driver_write_buffer(IPDriver* driver, uint32_t sector_number, const void* buffer) {
    if (!driver || !driver->img_file || !buffer) return -1;
    
    if (fseek(driver->img_file, sector_number * driver->sector_size, SEEK_SET) != 0) return -1;
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
echo -e "${YELLOW}3. Copying source files...${NC}"

# HAL
[ -d "../src/hal" ] && cp -r ../src/hal/* src/hal/

# FAT Driver
[ -d "../src/fat_driver" ] && cp -r ../src/fat_driver/* src/fat_driver/

# Middleware
[ -d "../src/middleware" ] && cp -r ../src/middleware/* src/middleware/

# Application
[ -d "../src/application" ] && cp -r ../src/application/* src/application/

# Utilities
[ -d "../src/utilities/log" ] && cp -r ../src/utilities/log/* src/utilities/log/
[ -d "../src/utilities/linkedlist" ] && cp -r ../src/utilities/linkedlist/* src/utilities/linkedlist/

# Common
[ -d "../src/common" ] && cp -r ../src/common/* src/common/

# Main
[ -f "../src/main.c" ] && cp ../src/main.c src/

# Tạo Makefile
echo -e "${YELLOW}4. Creating Makefile...${NC}"
cat > Makefile << EOF
# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -I./src
LDFLAGS = 

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

# Tool detection
ifneq (\$(OS),Windows_NT)
    MAKE_CMD = make
else
    MAKE_CMD = mingw32-make
endif

# Source files
SRCS = \$(wildcard \$(SRC_DIR)/*.c) \\
       \$(wildcard \$(SRC_DIR)/ip_driver/*.c) \\
       \$(wildcard \$(SRC_DIR)/hal/*.c) \\
       \$(wildcard \$(SRC_DIR)/fat_driver/*.c) \\
       \$(wildcard \$(SRC_DIR)/middleware/*.c) \\
       \$(wildcard \$(SRC_DIR)/application/*.c) \\
       \$(wildcard \$(SRC_DIR)/utilities/log/*.c) \\
       \$(wildcard \$(SRC_DIR)/utilities/linkedlist/*.c)

# Object files
OBJS = \$(SRCS:\$(SRC_DIR)/%.c=\$(OBJ_DIR)/%.o)

# Target
TARGET = \$(BIN_DIR)/fat_filesystem

all: directories \$(TARGET)

directories:
	@mkdir -p \$(BIN_DIR)
	@mkdir -p \$(OBJ_DIR)/ip_driver
	@mkdir -p \$(OBJ_DIR)/hal
	@mkdir -p \$(OBJ_DIR)/fat_driver
	@mkdir -p \$(OBJ_DIR)/middleware
	@mkdir -p \$(OBJ_DIR)/application
	@mkdir -p \$(OBJ_DIR)/utilities/log
	@mkdir -p \$(OBJ_DIR)/utilities/linkedlist

\$(TARGET): \$(OBJS)
	\$(CC) \$(OBJS) -o \$(TARGET) \$(LDFLAGS)

\$(OBJ_DIR)/%.o: \$(SRC_DIR)/%.c
	@mkdir -p \$(dir \$@)
	\$(CC) \$(CFLAGS) -c \$< -o \$@

clean:
	rm -rf \$(OBJ_DIR) \$(BIN_DIR)

rebuild: clean all

.PHONY: all clean rebuild directories
EOF

echo -e "${GREEN}Standarization complete!${NC}"