CC = gcc

KERNEL_DIR = ../../FreeRTOS-Kernel

TARGET = build/week03

CFLAGS = -std=gnu11 -Wall -Wextra -O0 -g -pthread

INCLUDES = \
	-Iinclude \
	-I$(KERNEL_DIR)/include \
	-I$(KERNEL_DIR)/portable/ThirdParty/GCC/Posix \
	-I$(KERNEL_DIR)/portable/ThirdParty/GCC/Posix/utils

SOURCES = \
	src/main.c \
	$(KERNEL_DIR)/tasks.c \
	$(KERNEL_DIR)/queue.c \
	$(KERNEL_DIR)/list.c \
	$(KERNEL_DIR)/timers.c \
	$(KERNEL_DIR)/event_groups.c \
	$(KERNEL_DIR)/stream_buffer.c \
	$(KERNEL_DIR)/portable/ThirdParty/GCC/Posix/port.c \
	$(KERNEL_DIR)/portable/ThirdParty/GCC/Posix/utils/wait_for_event.c \
	$(KERNEL_DIR)/portable/MemMang/heap_4.c

all:
	mkdir -p build
	$(CC) $(CFLAGS) $(INCLUDES) $(SOURCES) -o $(TARGET) -pthread

run: all
	./$(TARGET)

clean:
	rm -rf build