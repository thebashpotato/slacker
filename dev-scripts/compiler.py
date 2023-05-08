"""
Compiler module
"""

import settings
from utils import Log, Shell


class Compiler:
    """
    @desc a very light wrapper around cmake compiler type,
    its mostly just to make command line compiling a bit easier and less cumbersome.
    @param: cmake program name
    @param: cmake_flags build flags for cmake
    """

    def __init__(self, cmake: str, cmake_flags: str):
        self._shell = Shell()
        self._cpus: str = self._shell.execute_with_output("nproc")
        self._cmake: str = cmake
        self._cmake_flags: str = cmake_flags

    def __init_build(self) -> bool:
        """
        @desc
        @return True if the command succeeds, False if otherwise
        """
        return self._shell.execute(
            f"{self._cmake} -B {settings.PROJECT_BUILD_DIR.stem} {self._cmake_flags}")

    def __make(self) -> bool:
        """
        @desc compile the compared build
        """
        return self._shell.execute(f"{self._cmake} --build {settings.PROJECT_BUILD_DIR.stem} -- -j{self._cpus}")

    def compile(self) -> bool:
        """
        @desc
        @returns True if compiling succeeds, false otherwise
        """
        Log.info("Configuring the project..")
        if not settings.PROJECT_BUILD_DIR.exists():
            if not self.__init_build():
                return False
            if not self.__make():
                return False
            return True

        # if the build dir already exists we are just going to recompile
        Log.warn(
            f"{settings.PROJECT_BUILD_DIR} already exists, re-compiling with existing settings")
        return self.__make()
