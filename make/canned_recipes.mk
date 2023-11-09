# Embeds the window manager in a Xephyr window for testing and debugging
# The below command will need to be ran if you are attaching a debugger.
# echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
define _embed =
	echo "Embedding ${1}"
	if [ ! -f $(BUILD_DIR)/bin/${1} ]; then
		echo "Please build ${1} first"
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
	DISPLAY=:1 $(BUILD_DIR)/bin/${1} &
	wait $$XEPHYR_PID
endef


define _clean =
	if [ -d $(BUILD_DIR) ]; then
		rm -rf $(BUILD_DIR)
	fi

	if [ -f compile_commands.json ]; then
		rm compile_commands.json
	fi

	if [ -d .cache ]; then
		rm -rf .cache
	fi
	echo "All build artifacts have been removed"
endef


define _init =
	if command -v apt 1>/dev/null 2>&1; then
		sudo apt-get install libx11-dev libxft-dev picom feh dunst network-manager volumeicon-alsa
	else
		echo "You are not on a Debian based system, make a pull request for your package manager"
	fi
endef


define _init_dev =
	if command -v apt 1>/dev/null 2>&1; then
		sudo apt-get install libx11-dev libxft-dev bear clang clangd clang-format xserver-xephyr
	else
		echo "You are not on a Debian based system, make a pull request for your package manager"
	fi
endef


define _install =
	echo
	echo "Installing swm..."
	echo
	if [ ! -f $(BUILD_DIR)/bin/swm ]; then
		echo "Please build the project first"
		exit 0
	fi

	if [ ! -d $(SLACKER_DIR) ]; then
		mkdir -p $(SLACKER_DIR)
	fi

	if [ ! -d $(CONFIG_DIR)/picom ]; then
		mkdir -p $(CONFIG_DIR)/picom
	fi

	sudo install -v -Dm755 $(BUILD_DIR)/bin/swm $(DESTDIR)$(PREFIX)/bin
	sudo install -v -Dm755 $(PROJECT_ROOT)/install-files/autostart.sh $(DESTDIR)$(SLACKER_DIR)/
	sudo install -v -D $(PROJECT_ROOT)/install-files/background.jpg $(DESTDIR)$(SLACKER_DIR)/
	sudo install -v -D $(PROJECT_ROOT)/install-files/swm.desktop $(DESTDIR)/usr/share/xsessions/
	install -v $(PROJECT_ROOT)/install-files/picom/picom.conf $(CONFIG_DIR)/picom/
endef


define _uninstall =
	rm -vf $(DESTDIR)$(PREFIX)/bin/swm
	rm -vrf $(DESTDIR)$(PREFIX)/share/slacker
	rm -vf $(DESTDIR)/usr/share/xsessions/swm.desktop
endef
