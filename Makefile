CC := gcc
CCFLAGS := 
LDFLAGS := -lpthread
SRC ?= main.c
OUT ?= terminal-game

TARGET_DIR ?= builds
BINDIR := /usr/local/bin

MODE ?= default
DBG := -g -Wall
OPT := -O3

ifeq ($(MODE), debug)
	CCFLAGS += $(DBG)
else ifeq ($(MODE), release)
	CCFLAGS += $(OPT)
endif

build:
	$(CC) $(SRC) -o $(OUT) $(CCFLAGS) $(LDFLAGS)

	@if [ "$(MODE)" = "release" ]; then \
		sudo mv -f $(OUT) $(BINDIR)/$(OUT); \
		printf "sudo mv -f $(OUT) $(BINDIR)/$(OUT)\n"; \
	else \
		mkdir -p $(TARGET_DIR); \
		mv -f $(OUT) $(TARGET_DIR)/$(OUT); \
		printf "mkdir -p $(TARGET_DIR)\nmv -f $(OUT) $(TARGET_DIR)/$(OUT)\n"; \
	fi


clean: 
	sudo rm -f $(BINDIR)/$(OUT)

	@printf "Warning: This action removes the whole $(TARGET_DIR) folder! Are you sure? (y/n)\n"
	@read answer; \
	if [ "$$answer" = "y" ]; then \
		sudo rm -rf $(TARGET_DIR); \
		printf "\n"; \
	else \
		printf "Action cancelled.\n"; \
	fi

help:
	@printf "help: prints this message.\n"
	@printf "build: compiles source code.\n"
	@printf "clean: uninstalls program from bin (and current directory if you want).\n"

.PHONY: help clean install build debug-mode uninstall
