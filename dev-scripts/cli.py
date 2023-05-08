"""
Command line interface code goes here
"""

# standard lib
import argparse

# local package
import settings


class Cli:
    """
    @description Wraps the built-in argparse library
    """

    def __init__(self):
        self.__parser = argparse.ArgumentParser(
            prog="devkit",
            usage="%(prog)s [options]",
            description="A development tool for common cmake/C++ development tasks",
            epilog="Author: Matt Williams <matt.k.williams@protonmail.com>",
            allow_abbrev=False,
        )

        # create subcommand parser
        sub_commands = self.__parser.add_subparsers(
            dest='command',
            help="Available subcommands")

        # create the clang tooling commands
        clang_commands = sub_commands.add_parser(
            "clang",
            help="Execute linting and static analyzing with clang tooling")

        clang_commands.add_argument(
            "-f",
            "--format",
            action="store_true",
            help="Formats specified projects recursively via clang-format (files and directories can be ignored in the dev-scripts/settings.py file)")

        clang_commands.add_argument(
            "-t",
            "--tidy",
            action="store_true",
            help="Statically analyzes specified projects via clang-tidy (files and directories can be ignored in the dev-scripts/settings.py file)")

        # create the packager commands
        packager_commands = sub_commands.add_parser(
            "package",
            help="Build a debian, slackware, or rpm package for C/C++ software that uses the CMake build system")

        packager_commands.add_argument("-n",
                                       "--name",
                                       type=str,
                                       required=True,
                                       help="The name of the software you are packaging")

        packager_commands.add_argument(
            "-v",
            "--version",
            type=str,
            required=True,
            help="The current version of the software you are packaging",
        )

        packager_commands.add_argument(
            "-t",
            "--type",
            type=str,
            default="debian",
            help="Choose the packaging system. Can be on of 'slackware', 'debian, or 'rpm', defaults to debian"
        )

        packager_commands.add_argument(
            "-b",
            "--build-dir",
            type=str,
            default=f"{settings.PROJECT_BUILD_DIR}",
            help="path to the build directory of your CMake build, defaults to the current working directories build folder"
        )

        # set up compiler commands
        build_commands = sub_commands.add_parser(
            "build",
            help="Compiles the entire project using cmake configuration in settings.py")

        build_commands.add_argument(
            "--release",
            action="store_true",
            default=True,
            help="Build in release mode (no tests and no examples)")

        build_commands.add_argument(
            "--develop",
            action="store_true",
            help="Build in develop mode with debug flags (tests and examples are turned on)")

        # parse all the arguments
        self.__args = self.__parser.parse_args()

    @property
    def subcommand_package(self) -> bool:
        """
        @description Return true if the package command was used
        """
        return str(self.__args.command) == "package"

    @property
    def subcommand_clang(self) -> bool:
        """
        @description Return true if the clang command was used
        """
        return str(self.__args.command) == "clang"

    @property
    def subcommand_build(self) -> bool:
        """
        @description Return true if the build command was used
        """
        return str(self.__args.command) == "build"

    @property
    def args(self) -> argparse.Namespace:
        """
        @description: Return the argparse object
        """
        return self.__args

    @property
    def print_help(self):
        """
        @description: wrapper around argparses show help command
        """
        self.__parser.print_help()
