#!/bin/env python3

"""
Author: Matt Williams <matt.k.williams@protonmail.com>
Gitlab: https://gitlab.com/thebashpotato
Main executable file, all modules are utilized here
"""

# standard lib
import sys
from pathlib import Path

# local package
import settings
from tooling import Clang
from cli import Cli
from compiler import Compiler
from packager import Packager
from utils import Log, Shell


def preflight() -> None:
    """
    @desc run some preflight checks to make sure devkit won't run into issues
    """
    # make sure we are running linux, because what else would you run?
    if "linux" != sys.platform:
        raise Exception(f"{sys.platform} is currently not supported\n")

    shell = Shell()
    # verify that all the required programs are installed on the system
    for key, value in settings.CMAKE_PROGRAMS.items():
        program_name = value["name"]
        if program_name:
            if not shell.is_installed(program_name):
                raise Exception(
                    f"{program_name} was not found on the system, please install, or change the value of {key} in the tooling/settings.py file")
        else:
            raise Exception(
                f"Value for {key} is empty, please edit the value in tooling/settings.py file")


def main() -> int:
    """
    @desc main function
    """
    try:
        preflight()

        cli = Cli()

        if cli.subcommand_package:
            packager = Packager(Path(cli.args.build_dir),
                                cli.args.name, cli.args.version, cli.args.type)
            packager.build_pkg()
        elif cli.subcommand_clang:
            clang = Clang(settings.CMAKE_PROGRAMS["CLANG_FORMATTER"],
                          settings.CMAKE_PROGRAMS["CMAKE_FORMATTER"],
                          settings.CMAKE_PROGRAMS["CLANG_ANALYZER"])
            if cli.args.format:
                clang.format()
            if cli.args.tidy:
                clang.tidy()
        elif cli.subcommand_build:
            cmake: str = ""
            cmake_flags: str = ""
            if cli.args.release:
                cmake = settings.CMAKE_PROGRAMS["CMAKE_RELEASE"]["name"]
                cmake_flags = settings.CMAKE_PROGRAMS["CMAKE_RELEASE"]["flags"]
            if cli.args.develop:
                cmake = settings.CMAKE_PROGRAMS["CMAKE_DEVELOP"]["name"]
                cmake_flags = settings.CMAKE_PROGRAMS["CMAKE_DEVELOP"]["flags"]
            compiler = Compiler(cmake, cmake_flags)
            if not compiler.compile():
                return 1
        else:
            cli.print_help
            return 1
    except Exception as e:
        Log.error(f"{e}")
        return 1
    except KeyboardInterrupt:
        Log.warn("Ctrl + c requested..")
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
