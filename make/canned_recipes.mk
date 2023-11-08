define _setup =
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(DIST_DIR)
endef


# Embeds the window manager in a Xephyr window for testing and debugging
# The below command will need to be ran if you are attaching a debugger.
# echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
define _embed =
	if [ ! -f $(BIN_DIR)/$(TARGET) ]; then
		echo "Please build the window manager first"
		exit 0
	fi

	if ! command -v Xephyr 1>/dev/null 2>&1; then
		echo "Please install Xephyr"
		exit 0
	fi

	echo "Press Ctrl+Shift to grab mouse and keyboard events in the Xephyr window, and Ctrl+Shift again to release"

	Xephyr :1 -ac -br -noreset -screen 1000x600 &
	XEPHYR_PID=$!
	sleep 1
	DISPLAY=:1 ./$(BIN_DIR)/$(TARGET) &
	wait $$XEPHYR_PID
endef


define _format =
	if command -v clang-format 1>/dev/null 2>&1; then
		clang-format -i --verbose $(SRC_DIR)/*.c $(SRC_DIR)/*.h
	else
		echo "Please install clang-format"
	fi
endef


define _clean =
	if [ -d $(BUILD_DIR) ]; then
		rm -vrf $(BUILD_DIR)
	fi

	if [ -f compile_commands.json ]; then
		rm -v compile_commands.json
	fi

	if [ -d .cache ]; then
		rm -vrf .cache
	fi
endef


define _init =
	if command -v apt 1>/dev/null 2>&1; then
		apt-get install libx11-dev libxft-dev picom feh dunst network-manager volumeicon-alsa
	else
		echo "You are not on a Debian based system, make a pull request for your package manager"
	fi
endef


define _init_dev =
	if command -v apt 1>/dev/null 2>&1; then
		apt-get install libx11-dev libxft-dev bear clang clangd clang-format xserver-xephyr
	else
		echo "You are not on a Debian based system, make a pull request for your package manager"
	fi
endef


define __install =
	echo "Not implemented yet"
endef
