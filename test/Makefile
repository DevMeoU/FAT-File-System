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

CFLAGS    += -DTEST_DRIVER

CUR_DIR   = .
SRC_DIR   = ../project/src
BUILD_DIR = build
OBJ_DIR   = obj
BIN_DIR   = bin

SOURCES := $(wildcard $(SRC_DIR)/*/*.c)
SOURCES += $(wildcard $(SRC_DIR)/utilities/linkedlist/*.c)
SOURCES += $(wildcard $(CUR_DIR)/src/*.c)
SOURCES := $(filter-out $(SRC_DIR)/application/application.c, $(SOURCES))

OBJECTS := $(filter-out $(CUR_DIR)/src/%.c, $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/$(OBJ_DIR)/%.o, $(SOURCES)))
OBJECTS += $(patsubst $(CUR_DIR)/src/%.c, $(BUILD_DIR)/$(OBJ_DIR)/%.o, $(filter $(CUR_DIR)/src/%.c, $(SOURCES)))

TARGET  := $(BUILD_DIR)/$(BIN_DIR)/$(PROJECT_NAME).exe

all: $(TARGET)

$(TARGET): $(filter-out $(BUILD_DIR)/$(OBJ_DIR)/$(PROJECT_NAME).o, $(OBJECTS))
	$(CC) $(CFLAGS) -c $(CUR_DIR)/src/$(PROJECT_NAME).c -o $(BUILD_DIR)/$(OBJ_DIR)/$(PROJECT_NAME).o
	@mkdir -p $(BUILD_DIR)/$(BIN_DIR)
	$(CC) $(filter-out $(BUILD_DIR)/$(OBJ_DIR)/$(PROJECT_NAME).o, $(OBJECTS)) $(BUILD_DIR)/$(OBJ_DIR)/$(PROJECT_NAME).o -o $@

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
