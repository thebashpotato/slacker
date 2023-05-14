# Slacker

[![standard-readme compliant](https://img.shields.io/badge/standard--readme-OK-green.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

Slacker is a tiling window manager environment for X written in C++.
It consists of a screen_id locker utility, a dynamic tiling window manager,
and shared library which wraps the needed X11 functionality in a safe modern C++ 20
API. **It is currently in the very early stages and nothing works**

## Table of Contents

- [Background](#background)
- [Install](#install)
- [Usage](#usage)
- [Docs](#docs)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [License](#license)

## Background

Slacker was born out of my love for **dwm** and window managers, and my annoyance with the needless code golf
and spaghetti design of Suckless code. I also really do not enjoy the C programming language.
Some people may ask why not Rust? And why not Penrose? The answer is simple, Rust is a great language and Penrose
is nice, but I just enjoy writing C++ as crazy as that sounds. Therefor most of this code is a port of Dwm source code.
I used **dwm**, **slock** and other Suckless utilities to learn X client side programming.

### Project Goals

1. Build a clean implementation (to the best of my ability) of Dwm in C++ with all my favorite patches as modules rather
   than old school patches.

2. Serialize/Deserialize configuration into a json formatted config file, support hot-reloading for all tools in this
   repo.

3. Have well commented and documented code, with supporting unit tests and examples in each repository. Object
   composition will be favoured over multiple inheritance, the only time inheritance will be used is when implementing a
   pure virtual class as an interface for the sake of polymorphism.

4. Port as many Suckless tools (that I use) to C++ as possible. This repo will be monolithic,
   all scripts and dotfile configurations will be housed here as well, Arch linux is a first class citizen.

### Project Non Goals

1. This project is for me, It's not written in a new cool hipster language like Nim, Zig, Go or Rust, so deal with it
   or go learn how to program real things your self in whatever language you see fit.

2. I will make no attempt to meet a Source Lines of Code criteria, Slacker will get as big as needs to be,
   if you're here to bitch about lines of code, I suggest you take a look at the *SLOC* of your
   kernel before opening your mouth.

3. There will be no attempt to support Wayland, not because I think it is bad, Wayland is the future for good reason.
   Programming my own X tiling windowing environment and tools is something I've been wanting to do for a long time,
   simple as that.

## Install

The Root [Makefile](./Makefile) controls compiling. Running `make list` will show the commands.
`make dev` builds a debug version of all binaries and shared libraries. There should be no reason to execute cmake commands
directly in the terminal.

TODO
```

```

## Usage

TODO

```
```

## Docs

In-depth documentation for each source module can be found in the following markdown files in the *docs*
folder.

1. [libslacker](./docs/libslacker.md) C++ shared library which wraps `X11` and all subsequent `X libraries`.

2. [slacker-lock](./docs/slacker-wm.md) C++ screen_id locker utility, spiritual port of `slock`

3. [slacker-wm](./docs/slacker-wm.md) C++ dynamic tiling window manager, spiritual port of `dwm` with way more features.

4. [dev-scripts](./docs/dev-scripts.md) devkit program written in Python which aids in development.

5. [install-files](./docs/install-files.md) devkit program written in Python which aids in development.

## Maintainers

[@thebashpotato](https://github.com/thebashpotato)

## Contributing

See [the contributing file](CONTRIBUTING.md)!

PRs accepted.

Small note: If editing the README, please conform to
the [standard-readme](https://github.com/RichardLitt/standard-readme) specification.

## License

[GPLV3](./LICENSE) © 2023 Matt Williams
