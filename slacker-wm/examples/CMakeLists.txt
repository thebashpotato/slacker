#
# NOTE: add the executables
#
set(ScratchFile "${PROJECT_NAME}-scratch")

add_executable(${ScratchFile} "${APP_EXAMPLES_SOURCE_DIR}/scratch.cpp" ${UTILS_SOURCE_FILES})
target_include_directories(${ScratchFile} PUBLIC ${APP_INCLUDE_DIR})
target_link_libraries(${ScratchFile} PRIVATE slacker_project_options slacker_project_warnings stdc++fs)
