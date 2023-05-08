#
# NOTE: add the executables
#
set(ScratchFile "${PACKAGE_NAME}_scratch")

add_executable(${ScratchFile} "${APP_EXAMPLES_SOURCE_DIR}/scratch.cpp" ${UTILS_SOURCE_FILES})
target_include_directories(${ScratchFile} PUBLIC ${APP_INCLUDE_DIR})
target_link_libraries(${ScratchFile} PRIVATE project_options project_warnings stdc++fs)
