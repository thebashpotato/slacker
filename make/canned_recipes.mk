define _setup =
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(DIST_DIR)

	if [ ! -f $(SRC_DIR)/slacker_config.h ]; then
		cp -v $(SRC_DIR)/slacker_config.def.h $(SRC_DIR)/slacker_config.h
	fi
endef


# Embeds the window manager in a Xephyr window for testing and debugging
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

	Xephyr :1 -ac -br -noreset -screen 1024x768 &
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
		apt-get install libx11-dev libxft-dev libxinerama-dev picom feh dunst network-manager volumeicon-alsa
	else
		echo "You are not on a Debian based system, make a pull request for your package manager"
	fi
endef