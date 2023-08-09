# slacker

[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

[![C Build](https://github.com/thebashpotato/slacker/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/thebashpotato/slacker/actions/workflows/c-cpp.yml)

An X Tiling Window Manager for GNU/Linux and BSD's.

## Background

> Slacker is a hard fork of the Suckless software's Dwm. Some of the original code and
> ideas were kept, but it is heavily refactored, cleaned up, and makes an attempt to provide
> better documentation and explanations.

## Table of Contents

- [Install](#install)
- [Usage](#usage)
- [Development](#development)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Install

```bash
# Ubuntu based depenencies (Built in to the Makefile for ease of use)
`sudo make init`
```

## Usage

> The following make command will install all necessary files, log back in
> to your X environment choosing slacker as the Window Manager.

`sudo make install`

# Development

> The following utilities are what a C programmer using Makefiles targeting 
> the X11 environment will need for debugging and testing, and sanity.

1. `bear` => Generates a `compile_commands.json` file so your 
    language service provider will know what the hell is going on.

    - Install: `sudo apt install bear`
    - Usage: `bear -- make`

2. `Xephyr` => Allows the programmer to run the Window Manager in a nested X session
    so you don't have to log out , and back into the actual window manager.

    - Install: `sudo apt install xserver-xephyr`
    - Usage: `make embed`


## Maintainers

[@thebashpotato](https://github.com/thebashpotato)

## Contributing

PRs accepted.

Small note: If editing the README, please conform to the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

MIT © 2023 Matt Williams
