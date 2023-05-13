#
# NOTE: Output build param information
#
message(STATUS "Building ${PROJECT_NAME} [${CMAKE_BUILD_TYPE}]")

#
# NOTE: Target libslacker as a shared library
#
add_library(${PROJECT_NAME} SHARED ${ALL_SOURCE_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${APP_INCLUDE_DIR} ${X11_INCLUDE_DIR})
target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${X11_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PROJECT_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#
# NOTE: Generate install command for resulting Makefile
#
configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.in @ONLY)
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY "${APP_INCLUDE_DIR}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${CMAKE_BINARY_DIR}/${PROJECT_NAME}/src/${PROJECT_NAME}.pc"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig")
