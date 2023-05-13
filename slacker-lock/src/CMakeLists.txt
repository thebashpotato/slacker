#
# NOTE: Output build param information
#
message(STATUS "Building ${PROJECT_NAME} [${CMAKE_BUILD_TYPE}]")

#
# NOTE: Declare the executable and add all source modules
#
add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${APP_INCLUDE_DIR} ${X11_INCLUDE_DIR})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${LIB_SLACKER}
          ${X11_LIBRARIES})

#
# NOTE: Generate install command for resulting Makefile
#
install(TARGETS ${PROJECT_NAME})
