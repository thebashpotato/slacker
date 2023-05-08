#!/usr/bin/env bash
# ---
# Use "run program" to run it only if it is not already running
# Use "program &" to run it regardless
# ---
# NOTE: This script runs with every with every start up casewm
# TODO: run_once

function run() {
  if ! pgrep "$1" >/dev/null; then
    "$@" &
  fi
}

# Speed up X
xset r rate 200 60 &

# Make capslock behave like ctrl
setxkbmap -option ctrl:nocaps

# Set background
feh --bg-fill /usr/share/backgrounds/casewm-background.png

run picom
run nm-applet
run volumeicon
run dunst
pkill -fi gnome-keyring-daemon
eval "$(gnome-keyring-daemon --start --components=pkcs11,secrets,ssh)"