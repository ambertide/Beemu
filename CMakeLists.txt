cmake_minimum_required(VERSION 3.14)
project(Beemu VERSION 0.1 LANGUAGES C)

add_executable(Beemu src/beemu.c)
add_subdirectory(src)
target_include_directories(Beemu PRIVATE include)
target_include_directories(Beemu PUBLIC ${PROJECT_BINARY_DIR})

configure_file(include/internals/version.h.in version.h)