#
# NOTE: Output build param information
#
message(STATUS "Building ${PROJECT_NAME} [${CMAKE_BUILD_TYPE}]")

#
# NOTE: Declare the executable and add all source modules
#
add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES})

target_compile_options(${PROJECT_NAME} PUBLIC ${Fontconfig_COMPILE_OPTIONS})

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC ${APP_INCLUDE_DIR}
         ${X11_INCLUDE_DIR}
         ${FREETYPE_INCLUDE_DIRS}
         ${Fontconfig_INCLUDE_DIRS})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE slacker_project_options
          slacker_project_warnings
          stdc++fs
          ${LIB_SLACKER}
          ${X11_LIBRARIES}
          ${FREETYPE_LIBRARIES}
          ${Fontconfig_LIBRARIES})

#
# NOTE: Generate install command for resulting Makefile
#
install(TARGETS ${PROJECT_NAME})
