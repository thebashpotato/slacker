#
# NOTE: Output build param information
#
message(STATUS "Building ${PACKAGE_NAME} [${CMAKE_BUILD_TYPE}]")

#
# NOTE: Declare the executable and add all source modules
#
add_executable(${PACKAGE_NAME} ${ALL_SOURCE_FILES})

target_include_directories(${PACKAGE_NAME} PUBLIC ${APP_INCLUDE_DIR} ${X11_INCLUDE_DIR})
target_link_libraries(
  ${PACKAGE_NAME}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${LIB_SLACKER}
          ${X11_LIBRARIES})

#
# NOTE: Generate install command for resulting Makefile
#
install(TARGETS ${PACKAGE_NAME})
