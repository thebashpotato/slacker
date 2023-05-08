#
# NOTE: Output build param information
#
message(STATUS "Building ${PACKAGE_NAME} [${CMAKE_BUILD_TYPE}]")

#
# NOTE: Target libslacker as a shared library
#
add_library(${PACKAGE_NAME} SHARED ${ALL_SOURCE_FILES})

target_include_directories(${PACKAGE_NAME} PUBLIC ${APP_INCLUDE_DIR} ${X11_INCLUDE_DIR})
target_link_libraries(
  ${PACKAGE_NAME}
  PRIVATE project_options
          project_warnings
          stdc++fs
          ${X11_LIBRARIES})

set_target_properties(${PACKAGE_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(${PACKAGE_NAME} PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#
# NOTE: Generate install command for resulting Makefile
#
configure_file(${PACKAGE_NAME}.pc.in ${PACKAGE_NAME}.in @ONLY)
install(TARGETS ${PACKAGE_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY "${APP_INCLUDE_DIR}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES "${CMAKE_BINARY_DIR}/${PACKAGE_NAME}/src/${PACKAGE_NAME}.pc"
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig")
