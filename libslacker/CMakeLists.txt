set(PACKAGE_NAME ${PROJECT_NAME})

#
# NOTE: Set the name of the library in the top level `CMakeLists.txt` file
# This allows consuming binaries to to use `LIB_SLACKER` to link against
# when compiling in debugging mode locally, in other words libslacker is not installed
# globally on the system
#
set(LIB_SLACKER
    ${PACKAGE_NAME}
    PARENT_SCOPE)
#
# NOTE: Set our source and include directory variables for use through the build
#
set(APP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(APP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(APP_TEST_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/tests")
set(APP_EXAMPLES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/examples")

#
# NOTE: add additional project options
#
option(SLACKER_DEV_MODE "Enables testing and example builds for libslacker" ON)

#
# NOTE: Prevent in source builds (can't build in src/ or in project root)
#
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR OR CMAKE_CURRENT_BINARY_DIR STREQUAL APP_SOURCE_DIR)
  message(
    FATAL_ERROR
      "In source builds are not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

#
# NOTE: Declare sources
#
set(SLACKER_SOURCE_FILES
    "${APP_SOURCE_DIR}/display.cpp"
    "${APP_SOURCE_DIR}/window.cpp"
    "${APP_SOURCE_DIR}/version.cpp"
    "${APP_SOURCE_DIR}/geometry.cpp")
set(ALL_SOURCE_FILES ${SLACKER_SOURCE_FILES})

#
# NOTE: FindPackage and CPM
#
find_package(X11 REQUIRED)

#
# NOTE: Add source
#
add_subdirectory("src")

#
# NOTE: If dev mode is enabled
#
if(SLACKER_DEV_MODE)
  # Enable examples
  message(STATUS "${PACKAGE_NAME} -- Examples Enabled")
  add_subdirectory("examples")

  # Enable Unit Testing
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
    set(googletest_Installed
        ON
        PARENT_SCOPE)
  elseif(googletest_Installed)
    message(STATUS "gtest already installed")
  else()
    message(WARNING "gtest was not configured properly")
  endif()

  # add tests
  enable_testing()
  message(STATUS "${PACKAGE_NAME} -- Tests Enabled")
  add_subdirectory("tests")
endif()
