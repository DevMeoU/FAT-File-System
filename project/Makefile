# Makefile for FAT File System Project
# Author: Ducson9112k

#------------------
# Project Name
#------------------
PROJECT_NAME := application

#------------------
# Build Tools
#------------------
# Detect Operating System
ifeq ($(OS),Windows_NT)
    # Windows
    CC := gcc
    # Other Windows settings...
else
    # Linux/Unix
    CC := gcc
    # Other Linux settings...
endif

RM        := rm -rf
MKDIR     := mkdir -p

#------------------
# Directories
#------------------
ROOT_DIR  := .
SRC_DIR   := $(ROOT_DIR)/src
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR   := $(BUILD_DIR)/obj
BIN_DIR   := $(BUILD_DIR)/bin
DEP_DIR   := $(BUILD_DIR)/dep
IMAGE_DIR := $(ROOT_DIR)/images

#------------------
# Source Files
#------------------
# Find all .c files in src and subdirectories
SRCS := $(shell find $(SRC_DIR) -type f -name "*.c")

# Create object file list
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

# Create dependency file list
DEPS := $(SRCS:$(SRC_DIR)/%.c=$(DEP_DIR)/%.d)

#------------------
# Compiler Flags
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

# Add debug flags if needed
ifeq ($(DEBUG), ON)
CFLAGS += -g -DDEBUG
endif

# Add verbose flags if needed
ifeq ($(VERBOSE), ON)
CFLAGS += -v
endif

#------------------
# Target File
#------------------
TARGET := $(BIN_DIR)/$(PROJECT_NAME).exe

#------------------
# Image Files
#------------------
IMAGE_FILES := $(wildcard $(subst \\,/,$(IMAGE_DIR))/*.img)

# Number to access image
IMAGE_NUM ?= 1

# Select image file automatically (supports multiple files)
FILE_IMAGE := $(word $(IMAGE_NUM), $(IMAGE_FILES))

$(info Using image file: $(FILE_IMAGE))

#------------------
# Modes
#------------------
# Default modes
BUILD_MODE ?= release
MODE ?= read-only

#------------------
# Targets
#------------------
.PHONY: all clean run debug release help

# Default target
all: release

# Build release version (optimized)
release: CFLAGS += -O2 -DNDEBUG
release: $(TARGET)
	@echo "Build release completed"

# Build debug version
debug: CFLAGS += -g -DDEBUG
debug: $(TARGET)
	@echo "Build debug completed"

# Build executable
$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(OBJS) $(LDFLAGS) -o $@
	@echo "Build completed!"

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	@$(MKDIR) $(dir $@)
	@$(MKDIR) $(dir $(DEP_DIR)/$*.d)
	@$(CC) $(CFLAGS) -c $< -o $@

# Create necessary directories
$(BIN_DIR) $(OBJ_DIR):
	@$(MKDIR) $@

# Clean
clean:
	@echo "Cleaning..."
	@$(RM) $(BUILD_DIR)
	@echo "Clean completed!"

# Run
run: $(TARGET)
	@echo "Running $(TARGET)"
	@./$(TARGET) $(FILE_IMAGE) $(MODE)

# Print
print-%:
	@echo $* = $($*)

# Help
help:
	@echo "Available targets:"
	@echo "  all      - Build all (default)"
	@echo "  release  - Build release version"
	@echo "  debug    - Build debug version"
	@echo "  clean    - Remove build files"
	@echo "  run      - Run the program"
	@echo "  help     - Display this help"
	@echo "  print-%  - Print variable value"

# Include dependency files
-include $(DEPS)