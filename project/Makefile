# Makefile cho FAT File System Project
# Author: Ducson9112k

#------------------
# Tên project
#------------------
PROJECT_NAME := application

#------------------
# Công cụ build
#------------------
CC        := C:\cygwin64\bin\gcc.exe
RM        := rm -rf
MKDIR     := mkdir -p

#------------------
# Thư mục
#------------------
ROOT_DIR  := .
SRC_DIR   := $(ROOT_DIR)/src
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR   := $(BUILD_DIR)/obj
BIN_DIR   := $(BUILD_DIR)/bin
DEP_DIR   := $(BUILD_DIR)/dep

#------------------
# Các file nguồn
#------------------
# Tìm tất cả các file .c trong thư mục src và thư mục con
SRCS := $(shell find $(SRC_DIR) -name "*.c" ! -path "$(SRC_DIR)/template/*" ! -name "fat_driver_impl.c")

# Tạo danh sách file object tương ứng
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Tạo danh sách file dependency
DEPS := $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

#------------------
# Cờ biên dịch
#------------------
# Include paths
INC_DIRS := $(SRC_DIR) \
            $(SRC_DIR)/common \
            $(SRC_DIR)/ip_driver \
            $(SRC_DIR)/hal \
            $(SRC_DIR)/fat_driver \
            $(SRC_DIR)/middleware \
            $(SRC_DIR)/application \
            $(SRC_DIR)/utilities/linkedlist \
            $(SRC_DIR)/utilities/log

# Convert include directories to -I flags
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# Basic flags
CFLAGS := -Wall -Wextra -Werror \
          -Wno-unused-parameter \
          -Wno-unused-variable \
          -Wno-unused-function \
          -Wno-maybe-uninitialized \
          -std=c11 \
          -fdiagnostics-color=always \
          -g \
          -O2 \
          -MMD -MP \
          $(INC_FLAGS)

# Linker flags
LDFLAGS := -lm -lpthread -v

# Thêm cờ debug nếu cần
ifeq ($(DEBUG), ON)
CFLAGS += -g -DDEBUG
endif

# Thêm cờ verbose nếu cần
ifeq ($(VERBOSE), ON)
CFLAGS += -v
endif

#------------------
# File đích
#------------------
TARGET := $(BIN_DIR)/$(PROJECT_NAME)

#------------------
# Các target
#------------------
.PHONY: all clean run debug release help

# Target mặc định
all: release

# Build bản release (có tối ưu)
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)
	@echo "Build release hoàn thành"

# Build bản debug
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)
	@echo "Build debug hoàn thành"

# Build file thực thi
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(OBJS) $(LDFLAGS) -o $@
	@echo "Build hoàn thành!"

# Compile các file nguồn
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	@$(MKDIR) $(dir $@)
	@$(MKDIR) $(dir $(DEP_DIR)/$*.d)
	@$(CC) $(CFLAGS) -c $< -o $@

# Tạo các thư mục cần thiết
$(BIN_DIR) $(OBJ_DIR):
	@$(MKDIR) $@

# Clean
clean:
	@echo "Cleaning..."
	@$(RM) $(BUILD_DIR)
	@echo "Clean hoàn thành!"

# Run
run: $(TARGET)
	@echo "Running $(TARGET)..."
	@./$(TARGET)

# Help
help:
	@echo "Các target có sẵn:"
	@echo "  all      - Build tất cả (mặc định)"
	@echo "  release  - Build bản release"
	@echo "  debug    - Build bản debug"
	@echo "  clean    - Xóa các file build"
	@echo "  run      - Chạy chương trình"
	@echo "  help     - Hiển thị help này"

# Include dependency files
-include $(DEPS)