cmake_minimum_required(VERSION 3.10)

project(runcat LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(APPINDICATOR REQUIRED appindicator3-0.1)
include_directories(${APPINDICATOR_INCLUDE_DIRS})

add_executable(runcat main.c)

target_link_libraries(runcat ${APPINDICATOR_LIBRARIES})

set(ICONS 
  ./icons/cat/my-sleeping-symbolic.svg
  ./icons/cat/my-running-0-symbolic.svg
  ./icons/cat/my-running-1-symbolic.svg
  ./icons/cat/my-running-2-symbolic.svg
  ./icons/cat/my-running-3-symbolic.svg
  ./icons/cat/my-running-4-symbolic.svg
)
install(TARGETS runcat RUNTIME DESTINATION bin)
INSTALL(FILES ${ICONS} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/runcat/icons/cat/)

