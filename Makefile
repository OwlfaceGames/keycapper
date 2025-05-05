
# Compiler and flags for Mac with Homebrew
CC = gcc
CFLAGS = -Wall -std=c99 -ObjC

# Determine Homebrew prefix (handles both Intel and Apple Silicon Macs)
BREW_PREFIX := $(shell brew --prefix)

# Add Homebrew include paths
CFLAGS += -I$(BREW_PREFIX)/include

# Add Homebrew library paths and required libraries
LDFLAGS = -L$(BREW_PREFIX)/lib -lSDL2 -lSDL2_ttf -framework ApplicationServices -framework CoreFoundation

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Target executable
TARGET = keycapper

# Default target
all: $(BUILD_DIR) $(TARGET)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run the program
run: $(TARGET)
	./$(TARGET)

# Clean up
clean:
	rm -rf $(BUILD_DIR) $(TARGET)

.PHONY: all run clean

