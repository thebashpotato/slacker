#
# NOTE: determine required cmake here (use 'pip install cmake')
#
cmake_minimum_required(VERSION 3.18)

#
# NOTE: Set the C++ standard
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#
# NOTE: Set project name, and language.
#
project(
  slacker
  VERSION 0.1.0
  DESCRIPTION "Window Manager environment in modern C++"
  HOMEPAGE_URL "https://github.com/thebashpotato/slacker"
  LANGUAGES CXX C)

#
# NOTE: Add our custom cmake modules to the cmake module path
#
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#
# NOTE: Creates interfaced "libraries" for
# the custom StandardProjectSettings and CompilerWarnings.
# These libraries can then be linked to the tests, examples and
# release executables to ensure the correct c++ standard is used,
# and all defined warnings are used.
#
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
add_library(project_warnings INTERFACE)

#
# NOTE: Include custom cmake dev-scripts, and the CPM package manager
# and add compiler warnings from our projects cmake/ directory.
#
include(CPM)
include(StandardProjectSettings)
include(StaticAnalyzers)
include(CompilerWarnings)
include(GNUInstallDirs)
set_project_warnings(project_warnings)

#
# NOTE: Add sub-projects to the project
#
add_subdirectory("libslacker")
add_subdirectory("slacker-lock")
