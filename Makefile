# Author: Matt Williams <matt.k.williams@protonmail.com>
SHELL:=$(shell command -v sh)
DEVKIT:=python3 dev-scripts/devkit.py

define _list =
	cat << EOF
		==============================
		| Command   ||   Description |
		==============================
		list        --   Lists all available commands
		rel         --   Builds release mode with all optimization flags
		dev         --   Builds debugging mode with all unit tests and examples
		format      --   Recursively runs clang-format all legal cmake and source files
		tidy        --   Runs clang-tidy on on all legal source files
		tests       --   Runs ctest on the test suite
		install     --   Installs optimized binaries, libraries, files and scripts
		uninstall   --   Uninstalls all binararies, libraries, files and scripts
		clean       --   Cleans all build artifacts
	EOF
endef

define _uninstall =
	if [ ! -d build ] || [ ! -f build/install_manifest.txt ]; then
		echo "Cannot uninstall, no build folder found or no install_manifest.txt file found"
		echo "Try running => make install"
		exit 1
	else
		# xargs is linux only.
		echo "Removing installed files.."
		cat build/install_manifest.txt
		echo
	fi
endef

define _clean =
	echo "Cleaning build artifacts.."
	[ -d build ] && rm -rf build
	[ -d build-debug ] && rm -rf build-debug
	[ -d build-release ] && rm -rf build-release
	echo
endef

list:
	@$(call _list)

# Build in release mode, full optimizations, no debugging
rel:
	@${DEVKIT} build --release

# Build in debugging mode with all debugging symbols, unit tests and examples.
dev:
	@${DEVKIT} build --develop

# format recursively with clang-format
format:
	@${DEVKIT} clang --format

# Statically analyze with clang-tidy
tidy:
	@${DEVKIT} clang --tidy

tests: dev
	@cd build/app && ctest

install: rel
	@sudo cmake --install build

uninstall:
	@$(call _uninstall)

clean:
	@$(call _clean)

.ONESHELL:
.Phony: list rel dev format tidy tests install uninstall clean
