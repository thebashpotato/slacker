#
# NOTE: determine required cmake here (use 'pip install cmake')
#
cmake_minimum_required(VERSION 3.18)

#
# NOTE: Set workspace name, and language.
#
project(
  slacker-workspace
  VERSION 0.1.0
  DESCRIPTION "Window Manager environment in modern C++"
  HOMEPAGE_URL "https://github.com/thebashpotato/slacker"
  LANGUAGES CXX)

#
# NOTE: Set the C++ standard
#
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#
# NOTE: add additional project options
#
option(SLACKER_DEV_MODE "Enables testing and example builds for all projects" ON)

#
# NOTE: Add our custom cmake modules to the cmake module path
#
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

#
# NOTE: Include custom cmake dev-scripts, and the CPM package manager
# and add compiler warnings from our projects cmake/ directory.
#
include(CPM)
include(StandardProjectSettings)
include(StaticAnalyzers)
include(CompilerWarnings)
include(GNUInstallDirs)

#
# NOTE: Creates interfaced "libraries" for
# The custom StandardProjectSettings and CompilerWarnings.
# These libraries can then be linked to the tests, examples and
# release executables to ensure the correct c++ standard is used,
# and all defined warnings are used.
#
add_library(slacker_project_options INTERFACE)
target_compile_features(slacker_project_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})
add_library(slacker_project_warnings INTERFACE)
set_project_warnings(slacker_project_warnings)

#
# NOTE: FindPackage and CPM (Dependencies needed by all projects are defined at this level only.
#

# NOTE: https://cmake.org/cmake/help/latest/module/FindX11.html
find_package(X11 REQUIRED)

# NOTE: https://cmake.org/cmake/help/latest/module/FindFontconfig.html
find_package(Fontconfig REQUIRED)

# NOTE: https://cmake.org/cmake/help/latest/module/FindFreetype.html
find_package(Freetype REQUIRED)

if(SLACKER_DEV_MODE)
  # Enable examples
  set(CMAKE_BUILD_TYPE
      Debug
      CACHE STRING "Forcing build type to debug" FORCE)

  # NOTE: Bring in Gtest Unit testing library
  cpmaddpackage(
    NAME
    googletest
    GITHUB_REPOSITORY
    google/googletest
    GIT_TAG
    v1.13.0
    VERSION
    1.13.0
    OPTIONS
    "BUILD_GMOCK ON"
    "INSTALL_GTEST OFF"
    "gtest_force_shared_crt ON")
  if(googletest_ADDED)
    message(STATUS "gtest configured correctly")
  else()
    message(WARNING "gtest was not configured properly")
  endif()
  enable_testing()
else()
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Forcing build type to release" FORCE)
endif()

#
# NOTE: Add sub-projects to the project
#
add_subdirectory("libslacker")
add_subdirectory("slacker-wm")
add_subdirectory("slacker-lock")
