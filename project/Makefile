# app/Makefile
PROJECT_NAME = application

CC        = gcc
CFLAGS    = -Wall -Wextra \
            -I./src/ip_driver \
            -I./src/hal \
            -I./src/fat_driver \
            -I./src/middleware \
            -I./src/application \
            -I./src/utilities/linkedlist \
            -I./src/utilities/log

SRC_DIR   = src
OBJ_DIR   = obj
BIN_DIR   = bin
BUILD_DIR = build

SOURCES := $(wildcard $(SRC_DIR)/*/*.c) \
           $(wildcard $(SRC_DIR)/utilities/linkedlist/*.c)

OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/$(OBJ_DIR)/%.o, $(SOURCES))

TARGET  := $(BUILD_DIR)/$(BIN_DIR)/$(PROJECT_NAME).exe

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILD_DIR)/$(BIN_DIR)
	$(CC) $(OBJECTS) -o $@

$(BUILD_DIR)/$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	@./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)/$(OBJ_DIR) $(BUILD_DIR)/$(BIN_DIR)

print-%:
	@echo $* = $($*)

.PHONY: all clean