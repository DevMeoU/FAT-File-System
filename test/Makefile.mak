CC = gcc
CFLAGS = -Wall -Wextra -I./src/ip_driver -I./src/hal -I./src/fat_driver -I./src/middleware -I./src/application -I./src/utilities/linkedlist

SRCDIR = src
OBJDIR = obj
BINDIR = bin

SOURCES :=$(wildcard$(SRCDIR)/*/*.c)$(wildcard$(SRCDIR)/utilities/linkedlist/*.c)
OBJECTS :=$(patsubst$(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))
TARGET =$(BINDIR)/app.exe

all:$(TARGET)

$(TARGET):$(OBJECTS)
@mkdir -p $(BINDIR)
$(CC)$(OBJECTS) -o $(TARGET)

$(OBJDIR)/%.o:$(SRCDIR)/%.c
@mkdir -p $(dir$@)
$(CC)$(CFLAGS) -c $< -o $@

clean:
    rm -rf $(OBJDIR)$(BINDIR)