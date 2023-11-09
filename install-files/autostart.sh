#!/usr/bin/sh
# ---
# Use "run program" to run it only if it is not already running
# Use "program &" to run it regardless
# ---
# NOTE: This script runs with every with every start up swm
# TODO: run_once

run() {
  if ! pgrep "$1" >/dev/null; then
    "$@" &
  fi
}

# Speed up X
xset r rate 200 60 &

# Make capslock behave like ctrl
setxkbmap -option ctrl:nocaps

# Set background
feh --bg-fill /usr/local/share/slacker/background.jpg

run picom
#run nm-applet
#run volumeicon
#run dunst

# pkill -fi gnome-keyring-daemon
# eval "$(gnome-keyring-daemon --start --components=pkcs11,secrets,ssh)"
