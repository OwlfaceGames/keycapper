# Platform detection
ifeq ($(OS),Windows_NT)
	PLATFORM = WINDOWS
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		PLATFORM = MAC
	else
		PLATFORM = LINUX
	endif
endif

# Compiler and flags
CC = gcc
CFLAGS = -Wall -std=c99

ifeq ($(PLATFORM),MAC)
	# Mac with Homebrew
	BREW_PREFIX := $(shell brew --prefix)
	CFLAGS += -I$(BREW_PREFIX)/include
	LDFLAGS = -L$(BREW_PREFIX)/lib -lSDL2 -lSDL2_ttf -framework ApplicationServices -framework CoreFoundation
else ifeq ($(PLATFORM),WINDOWS)
	# Windows with direct dependency paths
	SDL2_DIR = C:/Users/nickj/keycapper_dependencies/SDL2
	SDL2_TTF_DIR = C:/Users/nickj/keycapper_dependencies/SDL2_ttf
	CFLAGS += -I$(SDL2_DIR)/include -I$(SDL2_TTF_DIR)/include
	LDFLAGS = -L$(SDL2_DIR)/lib/x64 -L$(SDL2_TTF_DIR)/lib/x64 -lSDL2 -lSDL2_ttf
else
	# Linux fallback (user must install dependencies)
	CFLAGS +=
	LDFLAGS = -lSDL2 -lSDL2_ttf
endif

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

