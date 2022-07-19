cmake_minimum_required(VERSION 3.14)
project(Beemu VERSION 0.1 LANGUAGES C)

add_library(libbeemu)
add_executable(Beemu src/beemu.c)
add_subdirectory(src)
add_subdirectory(libs)
target_include_directories(libbeemu PRIVATE include)
target_include_directories(Beemu PRIVATE include)
target_include_directories(libbeemu PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(Beemu PUBLIC libbeemu)

configure_file(include/version.h.in version.h)

# Executables

add_executable(BeemuTest tests/check_cpu.c)
target_link_libraries(BeemuTest PRIVATE unity)
target_link_libraries(BeemuTest PRIVATE libbeemu)
target_link_libraries(BeemuTest PRIVATE json-c)

enable_testing()

add_test(
	NAME BeemuTest
	COMMAND $<TARGET_FILE:BeemuTest>
)
