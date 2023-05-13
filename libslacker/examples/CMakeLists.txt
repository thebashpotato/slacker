#
# NOTE: add the executables
#
set(ScratchFile "${PROJECT_NAME}_scratch")
set(SimpleWindow "${PROJECT_NAME}_simple_window")

add_executable(${ScratchFile} "${APP_EXAMPLES_SOURCE_DIR}/scratch.cpp" ${SLACKER_SOURCE_FILES})
add_executable(${SimpleWindow} "${APP_EXAMPLES_SOURCE_DIR}/xsimplewindow.cpp" ${SLACKER_SOURCE_FILES})

target_include_directories(${ScratchFile} PUBLIC ${APP_INCLUDE_DIR})
target_include_directories(${SimpleWindow} PUBLIC ${APP_INCLUDE_DIR})

target_link_libraries(
  ${ScratchFile}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${X11_LIBRARIES})

target_link_libraries(
  ${SimpleWindow}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${X11_LIBRARIES})
