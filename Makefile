CC := gcc
SOURCE := main.c
TARGET_NAME := terminal-game
DEBUG_FLAGS := -g
BUILD_FLAGS := -pthread
RENAME_FLAG := -o
BUILD_DIR := ./build
BINDIR := /usr/local/bin


all: help

install: $(SOURCE)
	$(CC) $(BUILD_FLAGS) $(RENAME_FLAG) $(TARGET_NAME) $(SOURCE)
	sudo mv $(TARGET_NAME) $(BINDIR)/$(TARGET_NAME)

build: $(SOURCE)
	$(CC) $(BUILD_FLAGS) $(RENAME_FLAG) $(TARGET_NAME) $(SOURCE)

debug-mode: $(SOURCE)
	$(CC) $(BUILD_FLAGS) $(DEBUG_FLAGS) $(RENAME_FLAG) $(TARGET_NAME) $(SOURCE)

uninstall:
	sudo rm -f $(BINDIR)/$(TARGET_NAME)
	sudo rm -f $(TARGET_NAME)

help:
	@echo -e "No argument detected.\nExecuting 'make help'\n"
	@echo "install: compiles source code and copies it to $(BINDIR)" 
	@echo "build: compiles source code."
	@echo "debug: compiles source code with debug flags."
	@echo "uninstall: uninstalls program from $(BINDIR) and current directories."

.PHONY: help clean install build debug-mode uninstall