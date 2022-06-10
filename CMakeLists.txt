cmake_minimum_required(VERSION 3.14)
project(http_sfv)

set(CMAKE_VERBOSE_MAKEFILE on)

# Catch2 v3 requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

include_directories(
  include
)

file(GLOB SRC_FILES lib/*.c)
file(GLOB TEST_FILES tests/*.cpp)
add_executable(tests ${TEST_FILES} ${SRC_FILES})
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(tests)

