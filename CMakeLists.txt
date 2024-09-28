cmake_minimum_required(VERSION 3.14)
project(Beemu VERSION 0.1 LANGUAGES C CXX)
# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)

add_library(libbeemu)
add_executable(Beemu src/beemu.c)
add_subdirectory(src)
target_include_directories(libbeemu PRIVATE include)
target_include_directories(Beemu PRIVATE include)
target_include_directories(libbeemu PUBLIC ${PROJECT_BINARY_DIR})
target_link_libraries(Beemu PUBLIC libbeemu)

configure_file(include/version.h.in version.h)

# TESTING DEPENDENCIES START HERE

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

enable_testing()

add_subdirectory(tests)

target_include_directories(tests PRIVATE include)
