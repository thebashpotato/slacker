define _setup =
	if [ ! -d $(BUILD_DIR) ]; then
		mkdir -p $(BUILD_DIR)
	fi

	if [ ! -d $(OBJ_DIR) ]; then
		mkdir -p $(OBJ_DIR)
	fi

	if [ ! -d $(BIN_DIR) ]; then
		mkdir -p $(BIN_DIR)
	fi

	if [ ! -d $(DIST_DIR) ]; then
		mkdir -p $(DIST_DIR)
	fi
endef


define _format =
	if command -v clang-format 1>/dev/null 2>&1; then
		clang-format -i --verbose $(SRC_DIR)/*.c $(SRC_DIR)/*.h
	else
		echo "Please install clang-format"
	fi
endef