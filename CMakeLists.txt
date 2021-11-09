cmake_minimum_required(VERSION 3.10)

project(runcat LANGUAGES C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(APPINDICATOR REQUIRED appindicator3-0.1)
include_directories(${APPINDICATOR_INCLUDE_DIRS})

add_executable(runcat main.c)

target_link_libraries(runcat ${APPINDICATOR_LIBRARIES})

set(ICONS ./icons/)
install(DIRECTORY ${ICONS} DESTINATION $ENV{HOME}/.config/runcat/icons)

