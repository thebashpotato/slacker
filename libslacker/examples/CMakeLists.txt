#
# NOTE: add the executables
#
set(ScratchFile "${PROJECT_NAME}-scratch")
set(SimpleWindow "${PROJECT_NAME}-simple_window")

add_executable(${ScratchFile} "${APP_EXAMPLES_SOURCE_DIR}/scratch.cpp" ${SLACKER_SOURCE_FILES})
add_executable(${SimpleWindow} "${APP_EXAMPLES_SOURCE_DIR}/xsimplewindow.cpp" ${SLACKER_SOURCE_FILES})

target_include_directories(
  ${ScratchFile}
  PUBLIC ${APP_INCLUDE_DIR}
         ${X11_INCLUDE_DIR}
         ${FREETYPE_INCLUDE_DIRS}
         ${Fontconfig_INCLUDE_DIRS})

target_include_directories(
  ${SimpleWindow}
  PUBLIC ${APP_INCLUDE_DIR}
         ${X11_INCLUDE_DIR}
         ${FREETYPE_INCLUDE_DIRS}
         ${Fontconfig_INCLUDE_DIRS})

target_link_libraries(
  ${ScratchFile}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${X11_LIBRARIES}
          ${Fontconfig_LIBRARIES}
          ${FREETYPE_LIBRARIES})

target_link_libraries(
  ${SimpleWindow}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${X11_LIBRARIES}
          ${Fontconfig_LIBRARIES}
          ${FREETYPE_LIBRARIES})
