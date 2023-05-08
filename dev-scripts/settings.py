"""
Application wide settings are here
"""

from pathlib import Path
from typing import Dict, List

import distro

# Set the Project root directory
PROJECT_ROOT = Path(__file__).resolve(strict=True).parent.parent

# Set the build directory
PROJECT_BUILD_DIR = PROJECT_ROOT / "build"

# All c++ projects must be added to this list
PROJECTS: List[Path] = [
    PROJECT_ROOT / "slacker-lock",
    PROJECT_ROOT / "libslacker",
]

# Package configurations, used for building a debian, slackware, or rpm package,
# change your information accordingly
PACKAGE_CONFIG: Dict[str, str] = {
    "BACKEND": "checkinstall",
    "LICENSE": "GPL-3.0",
    "MAINTAINER": "thepashpotato",
    "REQUIRES": "",
    "RELEASE": f"{distro.codename()}-{distro.version()}"
}

# Cmake stuff

# Expected programs, here compilers are specified so different versions
# can easily be swapped in and out by editing this file
CMAKE_PROGRAMS: Dict[str, Dict[str, str]] = {
    # Build in release mode, modify the flags accordingly
    "CMAKE_RELEASE": {
        "name": "cmake",
        "flags": "-D CMAKE_BUILD_TYPE=Release -D SLACKER_LOCK_DEV_MODE=OFF -D SLACKER_DEV_MODE=OFF -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_C_COMPILER=clang",
    },
    # Build in develop mode, modify the flags accordingly, *_DEV_MODE is on by default so no need to specify
    "CMAKE_DEVELOP": {
        "name": "cmake",
        "flags": "-D CMAKE_BUILD_TYPE=Debug -D CMAKE_CXX_COMPILER=clang++ -D CMAKE_C_COMPILER=clang",
    },
    # customize formatting and clang tidy through the flags
    "CLANG_FORMATTER": {
        "name": "clang-format",
        "flags": "-i"
    },
    "CMAKE_FORMATTER": {
        "name": "cmake-format",
        "flags": "-i"
    },
    "CLANG_ANALYZER": {
        "name": "clang-tidy",
        "flags": f"-p {PROJECT_BUILD_DIR} -header-filter=.* --config-file={PROJECT_ROOT / '.clang-tidy.yaml'}"
    },
}

# Files that should be ignored for clang-format
CLANG_FORMAT_IGNORE_FILE: List[Path] = [
]

# usefull for 3rd party bundled projects, unit testing and example code
CLANG_FORMAT_IGNORE_DIR: List[Path] = [
]

# Files that should be ignored for clang-tidy
CLANG_TIDY_IGNORE_FILE: List[Path] = [
]

# usefull for 3rd party bundled projects, unit testing and example code
CLANG_TIDY_IGNORE_DIR: List[Path] = [
    PROJECT_ROOT / 'slacker-lock' / 'tests',
    PROJECT_ROOT / 'slacker-lock' / 'examples',
    PROJECT_ROOT / 'libslacker' / 'tests',
    PROJECT_ROOT / 'libslacker' / 'examples',
]
