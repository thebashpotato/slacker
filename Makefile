# Slacker X11 Environment Workspace Makefile.
# Dispatches commands to all lower level Makefiles.
# Pure makefile based build system written from scratch.. come at me bro.

PROJECT_ROOT=$(shell pwd)
BUILD_DIR:=$(PROJECT_ROOT)/build

include make/canned_recipes.mk
include make/settings.mk

swm:
	@$(MAKE) -C swm/ all BUILD_DIR=$(BUILD_DIR) CC=$(COMPILER) DEBUG=0 UNIT_TESTS=0

swm-dev: clean
	@bear -- $(MAKE) -C swm/ all BUILD_DIR=$(BUILD_DIR) CC=$(COMPILER) DEBUG=1 UNIT_TESTS=0

swm-debug: clean
	@bear -- $(MAKE) -C swm/ BUILD_DIR=$(BUILD_DIR) CC=$(COMPILER) DEBUG=1 UNIT_TESTS=0
	@$(call _embed,swm)

swm-tests: clean
	@bear -- $(MAKE) -C swm/ all BUILD_DIR=$(BUILD_DIR) CC=$(COMPILER) DEBUG=1 UNIT_TESTS=1
	@$(call _run_tests,swm)

format:
	@$(MAKE) -C swm/ format

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

.PHONY: swm swm-dev swm-debug swm-tests format init init-dev install uninstall clean

.ONESHELL:
