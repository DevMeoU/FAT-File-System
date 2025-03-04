# test/Makefile
PROJECT_NAME = test

CC        = gcc
CFLAGS    = -Wall -Wextra \
			-I./test/src \
            -I../project/src/ip_driver \
            -I../project/src/hal \
            -I../project/src/fat_driver \
            -I../project/src/middleware \
            -I../project/src/utilities/linkedlist

CUR_DIR   = .
SRC_DIR   = ../project/src
BUILD_DIR = build
OBJ_DIR   = obj
BIN_DIR   = bin

SOURCES := $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(CUR_DIR)/src/*.c)
SOURCES := $(filter-out $(SRC_DIR)/application/application.c, $(SOURCES))

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

.PHONY: all clean run print-%
