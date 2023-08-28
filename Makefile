# slacker - Fork of the orignal Dynamic Tiling Window Manager from suckless.org
# See LICENSE file for copyright and license details.

TARGET = slacker
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
BIN_DIR = $(BUILD_DIR)/bin
DIST_DIR = $(BUILD_DIR)/dist

include make/compiler_settings.mk
include make/canned_recipes.mk

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

all: options setup $(TARGET)

options:
	@echo ========================
	@echo $(TARGET) build options:
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo "CC      = $(CC)"
	@echo ========================

setup:
	@$(call _setup)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c  $< -o $@

$(TARGET): $(OBJ)
	$(CC) -o $(BIN_DIR)/$@ $(OBJ) $(LDFLAGS)

init:
	@$(call _init)

embed:
	@$(call _embed)

format:
	@$(call _format)

clean:
	@$(call _clean)


.PHONY: all options init embed clean

# This magic snippet from is what enables the programmer to write
# bash scripts in canned recipes, without all the annoying escapes.
.ONESHELL:
