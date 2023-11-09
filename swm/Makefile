# Slacker Window Manager. Suckless style non-reparenting window manager.

TARGET=swm
SRC_DIR=src
BUILD_DIR?=build
OBJ_DIR=$(BUILD_DIR)/$(TARGET)/obj
BIN_DIR=$(BUILD_DIR)/bin
DIST_DIR=$(BUILD_DIR)/dist
CC?=gcc

include make/swm_settings.mk
include make/canned_recipes.mk

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: options setup $(TARGET)

setup:
	@$(call _setup)

options:
	@echo ========================
	@echo $(TARGET) build options:
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"
	@echo ========================

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c  $< -o $@

$(TARGET): $(OBJ)
	@$(CC) -o $(BIN_DIR)/$@ $(OBJ) $(LDFLAGS)

format:
	@$(call _format)

# This magic snippet from is what enables the programmer to write
# bash scripts in canned recipes, without all the annoying escapes.
.ONESHELL:
