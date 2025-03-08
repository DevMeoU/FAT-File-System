# Makefile cho FAT File System Project
# Author: Ducson9112k

#------------------
# Tên project
#------------------
PROJECT_NAME := application

#------------------
# Công cụ build
#------------------
CC        := gcc
RM        := rm -rf
MKDIR     := mkdir -p

#------------------
# Thư mục
#------------------
SRC_DIR   := src
BUILD_DIR := build
OBJ_DIR   := $(BUILD_DIR)/obj
BIN_DIR   := $(BUILD_DIR)/bin
DEP_DIR   := $(BUILD_DIR)/dep

#------------------
# Các file nguồn
#------------------
# Tìm tất cả các file .c trong thư mục src và thư mục con
SRCS := $(filter-out $(SRC_DIR)/template/%, $(wildcard $(SRC_DIR)/*/*.c)) \
        $(wildcard $(SRC_DIR)/utilities/linkedlist/*.c)

# Tạo danh sách file object tương ứng
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Tạo danh sách file dependency
DEPS := $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

#------------------
# Cờ biên dịch
#------------------
CFLAGS := -Wall -Wextra -Werror \
          -O2 \
          -MMD -MP \
          -I$(SRC_DIR)/ip_driver \
          -I$(SRC_DIR)/hal \
          -I$(SRC_DIR)/fat_driver \
          -I$(SRC_DIR)/middleware \
          -I$(SRC_DIR)/application \
          -I$(SRC_DIR)/utilities/linkedlist \
          -I$(SRC_DIR)/utilities/log \
          -I$(SRC_DIR)/utilities/status

LDFLAGS := 

#------------------
# File đích
#------------------
TARGET := $(BIN_DIR)/$(PROJECT_NAME).exe

#------------------
# Các target
#------------------
.PHONY: all clean run debug release help

# Target mặc định
all: release

# Build bản release (có tối ưu)
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)
	@echo "Build release hoàn thành: $(TARGET)"

# Build bản debug
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET) 
	@echo "Build debug hoàn thành: $(TARGET)"

# Build file thực thi
$(TARGET): $(OBJS)
	@echo "Linking $@..."
	@$(MKDIR) $(dir $@)
	@$(CC) $(OBJS) $(LDFLAGS) -o $@
	@echo "Build hoàn thành!"

# Compile các file nguồn
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling $<..."
	@$(MKDIR) $(dir $@)
	@$(MKDIR) $(dir $(DEP_DIR)/$*.d)
	@$(CC) $(CFLAGS) -c $< -o $@

# Chạy chương trình
run: $(TARGET)
	@echo "Đang chạy $(TARGET)..."
	@./$(TARGET)

# Dọn dẹp
clean:
	@echo "Đang dọn dẹp..."
	@$(RM) $(BUILD_DIR)
	@echo "Đã xóa thư mục build!"

# Hiển thị trợ giúp
help:
	@echo "Các target có sẵn:"
	@echo "  all      - Build bản release (mặc định)"
	@echo "  debug    - Build bản debug có symbol"
	@echo "  release  - Build bản release đã tối ưu"
	@echo "  run      - Chạy chương trình"
	@echo "  clean    - Xóa các file đã build"
	@echo "  help     - Hiển thị trợ giúp này"

# In giá trị biến để debug Makefile
print-%:
	@echo $* = $($*)

# Include các file dependency
-include $(DEPS)