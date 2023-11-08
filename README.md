# Slacker X11 environment

[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)
[![C Build](https://github.com/thebashpotato/slacker/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/thebashpotato/slacker/actions/workflows/c-cpp.yml)

An X11 Tiling Window Manager for GNU/Linux and BSD's.

## Background

> Slacker Window Manager (swm) is a suckless-style X11 non-reparenting window manager.
> It is an attempt at creating a more structually sound and concise version of Dwm
> using straight forward C design patterns, refactoring as much spaghetti code as possible,
> and using explicit naming conventions. It also aims to ship out of the box with
> key features like a systray, status line etc..

## Table of Contents

- [Install](#install)
- [Development](#development)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Install

> Installing and using the window manager

```bash
# Installs the X11 libraries, dunst, volumeicon-alsa, network-manager, feh, picom,
# for apt-get based systems.
sudo make init

# Compiles the the software
make all

# Install's source code, desktop file, picom file, etc.
sudo make install
```

## Development

> Install the development tools

```bash
# Install all X11 libraries and development tools for apt-get based systems.
sudo make init-dev

# Run the window manager in an embedded X window for testing and development
make embed
```

> The following utilities are what a C programmer using Makefiles targeting
> the X11 environment will need for debugging and testing, and sanity. They are all
> installed with the above init-dev command.

1. `bear` => Generates a `compile_commands.json` file so your
    language service provider (clangd) will know what the hell is going on.
    - Usage: **bear -- make all**

2. `Xephyr` => Allows the programmer to run the Window Manager in a nested X session
    so you don't have to log out , and back into the actual window manager. Perfect for developing, debugging and testing.
    - Usage: **make embed**

3. `clang-format` => Formats all source code files according the the root .clang-format file.
    This project uses the Linux kernel clang-format.
    - Usage **make format**

4. `clangd` => Modern C/C++ code analyzing to editors.
    - Usage: Vim, Emacs, Vscode and Jetbrains all support it and handle it for you.

### Debugging

This repository includes a [Vscode launch.json](.vscode/launch.json) file for attaching a debugger
to `swm` while it's running inside an embedded Xephyr window. You will need to llvm debugging extension installed.
You will also need to run the following commands.

> This allows the necessary permissions needed for the debugger to attach.

`echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope`

> This is the compile command, there is a 15 second pause to give time to attach the debugger to the process

`make clean && bear -- make DEBUG=1 && make embed`

## Maintainers

[@thebashpotato](https://github.com/thebashpotato)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

MIT © 2023 Matt Williams
