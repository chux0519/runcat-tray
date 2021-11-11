cmake_minimum_required(VERSION 3.10)

project(runcat LANGUAGES C)

# AddressSanitize
set(CMAKE_C_FLAGS_ASAN
    "-fsanitize=address -fno-optimize-sibling-calls -fsanitize-address-use-after-scope -fno-omit-frame-pointer -g -O1"
    CACHE STRING "Flags used by the C compiler during AddressSanitizer builds."
    FORCE)



find_package(PkgConfig REQUIRED)
pkg_check_modules(APPINDICATOR REQUIRED appindicator3-0.1)
include_directories(${APPINDICATOR_INCLUDE_DIRS})

add_executable(runcat main.c)

target_link_libraries(runcat ${APPINDICATOR_LIBRARIES})

set(ICONS ./icons/)
install(DIRECTORY ${ICONS} DESTINATION $ENV{HOME}/.config/runcat/icons)

