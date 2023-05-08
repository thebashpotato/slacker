"""
Wrap up clang tooling functionality
"""

from enum import Enum
# standard lib
from pathlib import Path
from typing import Dict, List

# local package
import settings
from utils import Log, Shell


class Clang:
    class IgnoreType(Enum):
        ClangFormat = 0
        ClangTidy = 1

    def __init__(self, cxx_formatter: Dict[str, str], cmake_formatter: Dict[str, str], linter: Dict[str, str]):
        self._shell = Shell()
        self._cxx_formatter: str = cxx_formatter["name"]
        self._cxx_formatter_flags: str = cxx_formatter["flags"]
        self._cmake_formatter: str = cmake_formatter["name"]
        self._cmake_formatter_flags: str = cmake_formatter["flags"]
        self._linter: str = linter["name"]
        self._linter_flags: str = linter["flags"]

        # gather all cpp/hpp files for all projects
        self._source_files: List[Path] = []
        # gather all cmake files for formatting
        self._cmake_files: List[Path] = []
        # hold all ignored files for reporting at the end
        self._ignored_files: List[Path] = []

    def _aggregate_files(self, ignore_type: IgnoreType) -> None:
        """
        @desc Collect all legal files based on the type of tool that is being requested
        @param: Either clang-tidy or clang-format
        """
        for project in settings.PROJECTS:
            for file in project.rglob("**/*.cpp"):
                if self._path_in_file_ignore(file, ignore_type):
                    self._ignored_files.append(file)
                    continue
                if self._path_in_directory_ignore(file, project, ignore_type):
                    self._ignored_files.append(file)
                    continue
                self._source_files.append(file)
            for file in project.rglob("**/*.hpp"):
                if self._path_in_file_ignore(file, ignore_type):
                    self._ignored_files.append(file)
                    continue
                if self._path_in_directory_ignore(file, project, ignore_type):
                    self._ignored_files.append(file)
                    continue
                self._source_files.append(file)
            # Gather CMake files for each project
            for file in project.rglob("**/*.txt"):
                self._cmake_files.append(file)

        # Gather root level cmake files and modules
        for path in settings.PROJECT_ROOT.iterdir():
            if path.is_dir() and path.name == "cmake":
                for file in path.rglob("**/*.cmake"):
                    self._cmake_files.append(file)
            if path.is_file() and path.name == "CMakeLists.txt":
                self._cmake_files.append(path)

    @staticmethod
    def _path_in_file_ignore(path: Path, ignore_type: IgnoreType) -> bool:
        """
        @desc helper for ignoring only files
        """
        if ignore_type == Clang.IgnoreType.ClangFormat:
            if path in settings.CLANG_FORMAT_IGNORE_FILE:
                return True

        if ignore_type == Clang.IgnoreType.ClangTidy:
            if path in settings.CLANG_TIDY_IGNORE_FILE:
                return True

        return False

    def _path_in_directory_ignore(self, path: Path, project_root: Path, ignore_type: IgnoreType) -> bool:
        """
        @desc quick and dirty recursive method to test if a file is in a specified
        ignored directory.
        """
        # base case, stop recursion if the path has been chopped to the current project root,
        # we know the file isn't being ignored
        if path == project_root:
            return False

        if ignore_type == Clang.IgnoreType.ClangFormat:
            if path in settings.CLANG_FORMAT_IGNORE_DIR:
                return True

        if ignore_type == Clang.IgnoreType.ClangTidy:
            if path in settings.CLANG_TIDY_IGNORE_DIR:
                return True

        return self._path_in_directory_ignore(path.parent, project_root, ignore_type)

    def _show_stats(self, ignore_type: IgnoreType) -> None:
        """
        @desc print the number of ignored files
        """
        msg: str = ""
        ignored_msg: str = ""
        if ignore_type == Clang.IgnoreType.ClangFormat:
            msg += f"Formatted [{len(self._source_files) + len(self._cmake_files)}] file(s)"
            ignored_msg += f"Ignored: [{len(self._ignored_files)}] file(s) in [{len(settings.CLANG_FORMAT_IGNORE_DIR)}] directory(s)"
        else:
            msg += f"Statically Analyzed {len(self._source_files)} file(s)"
            ignored_msg += f"Ignored: [{len(self._ignored_files)}] file(s) in [{len(settings.CLANG_TIDY_IGNORE_DIR)}] directory(s)"

        print()
        Log.complete(msg)
        Log.warn(ignored_msg)

    def format(self):
        """
        @desc Run clang format on all the files found in self.files
        @raises Exception if name values are None or Empty
        """
        self._aggregate_files(Clang.IgnoreType.ClangFormat)
        for file in self._source_files:
            self._shell.execute(
                f"{self._cxx_formatter} {self._cxx_formatter_flags} {file}")

        print()
        for file in self._cmake_files:
            self._shell.execute(
                f"{self._cmake_formatter} {self._cmake_formatter_flags} {file}")

        self._show_stats(Clang.IgnoreType.ClangFormat)

    def tidy(self):
        """
        @desc Run clang-tidy on all legal files
        """
        self._aggregate_files(Clang.IgnoreType.ClangTidy)
        files: str = ""
        for file in self._source_files:
            files += f"{file} "
        self._shell.execute(f"{self._linter} {self._linter_flags} {files}")
        self._show_stats(Clang.IgnoreType.ClangTidy)
