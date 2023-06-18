#!/bin/bash

# The script launches Xephyr with display number :1,
# with the options:
#
# -ac (enabling access control),
# -br (enabling backing store),
# -noreset (not resetting the server when the last client disconnects),
# -screen 1024x768 (setting the screen resolution to 1024x768).
#
# The sleep 1 command gives Xephyr a second to start up before launching
# the xterm application on the new display.
# The DISPLAY=:1 command sets the DISPLAY environment variable to :1 so
# that the xterm application knows to use the new display.

if [[ -z "$1" ]]; then
  echo "Requires name of application to run"
  exit 1
fi

APP="$1"

Xephyr :1 -ac -br -noreset -screen 1024x768 &
XEPHYR_PID=$!
sleep 1
DISPLAY=:1 "$APP" &
wait $XEPHYR_PID
