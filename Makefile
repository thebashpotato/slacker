# Slacker X11 Environment Workspace Makefile.
# Dispatches commands to all lower level Makefiles.
# Pure makefile based build system written from scratch.. come at me bro.

PROJECT_ROOT=$(shell pwd)
BUILD_DIR:=$(PROJECT_ROOT)/build

include make/canned_recipes.mk
include make/settings.mk

swm:
	@make -C swm/ all BUILD_DIR=$(BUILD_DIR) CC=$(COMPILER)

swm-dev: clean
	@bear -- make -C swm/ all BUILD_DIR=$(BUILD_DIR) CC=$(COMPILER)

swm-debug: clean
	@bear -- make -C swm/ BUILD_DIR=$(BUILD_DIR) CC=$(COMPILER) DEBUG=1
	@$(call _embed,swm)

format:
	@make -C swm/ format

init:
	@$(call _init)

init-dev:
	@$(call _init_dev)

install: swm
	@$(call _install)

uninstall:
	@$(call _uninstall)

clean:
	@$(call _clean)

.PHONY: swm swm-dev swm-debug format init init-dev install uninstall clean

.ONESHELL:
