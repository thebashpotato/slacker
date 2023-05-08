"""
Uses checkinstall as a backend to create a debian package.
"""

# standard lib
from pathlib import Path
import os

# local package
from settings import PACKAGE_CONFIG
from utils import Log, Shell


class Packager:
    """
    A simple class that uses checkinstall to build
    a debian package
    """

    def __init__(self, build_dir: Path, pkg_name: str, pkg_version: str, pkg_type: str):
        if not build_dir.exists():
            raise Exception(
                f"Build directory {build_dir} does not exist, aborting...")
        # HACK workaround for docker container, as the $USER env variable is not defined
        self._sudo = ""
        user = os.getenv("USER")
        if user and user != "root":
            self._sudo = "sudo"

        self._shell = Shell()
        self._backend = PACKAGE_CONFIG["BACKEND"]
        if not self._shell.is_installed(self._backend):
            raise Exception(
                f"The packaging backend [{self._backend}] was not found on the system, please install and try again")
        self._license = PACKAGE_CONFIG["LICENSE"]
        self._maintainer = PACKAGE_CONFIG["MAINTAINER"]
        self._requires = PACKAGE_CONFIG["REQUIRES"]
        self._release = PACKAGE_CONFIG["RELEASE"]
        self._build_dir = build_dir
        self._name: str = pkg_name
        self._version: str = pkg_version
        self._type: str = pkg_type

        if not self.__configure_build_env():
            Log.error(f"{self._build_dir} is an invalid build directory")
        Log.info(f"Build directory resolved => {self._build_dir}")

    def __configure_build_env(self) -> bool:
        """
        @description: Make sure the build directory passed was valid
        @returns: True if a valid CMake build directory was found, False if not
        """
        # first check if the passed directory is correct
        found = False
        if "build" in self._build_dir.stem:
            found = True
        else:
            # the stem directory is not build, but it maybe up one more directory
            for receptacle in self._build_dir.iterdir():
                if receptacle.is_dir():
                    if "build" in receptacle.stem:
                        self._build_dir = receptacle
                        found = True
        return found

    def build_pkg(self) -> None:
        """
        @description: build the package with parameters that were fed to us
        """
        self._shell.execute(
            f"cd {str(self._build_dir)} && {self._sudo} {self._backend} -y --pkgname={self._name} --pkgrelease={self._release}"
            f" --pkgversion={self._version} --pkglicense={self._license} --requires={self._requires}"
            f" --maintainer={self._maintainer} --type={self._type} --install=no"
        )
