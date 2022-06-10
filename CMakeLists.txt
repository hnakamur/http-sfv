cmake_minimum_required(VERSION 3.14)
project(http_sfv)

set(CMAKE_VERBOSE_MAKEFILE on)

# Catch2 v3 requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.0.1 # or a later release
)

FetchContent_MakeAvailable(Catch2)

# clang-format
option(
  FORMAT_FILES_WITH_CLANG_FORMAT_BEFORE_EACH_BUILD
  "If the command clang-format is avilable, format source files before each build.\
Turn this off if the build time is too slow."
  ON)
find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-14
                                    clang-format-13 clang-format-12)
function(clang_format target)
  if(CLANG_FORMAT_EXE)
    message(STATUS "Enable Clang-Format ${target}")
    get_target_property(MY_SOURCES ${target} SOURCES)
    add_custom_target(
      "${target}_format-with-clang-format"
      COMMAND "${CLANG_FORMAT_EXE}" -i -style=file ${MY_SOURCES}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
    if(FORMAT_FILES_WITH_CLANG_FORMAT_BEFORE_EACH_BUILD)
      add_dependencies(${target} "${target}_format-with-clang-format")
    endif()
  else()
    message(STATUS "clang-format executable not found")
  endif()
endfunction()

# cmake-format
option(
  FORMAT_FILES_WITH_CMAKE_FORMAT_BEFORE_EACH_BUILD
  "If the command cmake-format is avilable, format source files before each build.\
Turn this off if the build time is too slow."
  ON)
find_program(CMAKE_FORMAT_EXE cmake-format clang-format-14)
function(cmake_format target)
  if(CMAKE_FORMAT_EXE)
    message(STATUS "Enable cmake-format")
    add_custom_target(
      "${target}_format-with-cmake-format"
      COMMAND "${CMAKE_FORMAT_EXE}" -i ${CMAKE_SOURCE_DIR}/CMakeLists.txt)
    if(FORMAT_FILES_WITH_CMAKE_FORMAT_BEFORE_EACH_BUILD)
      add_dependencies(${target} "${target}_format-with-cmake-format")
    endif()
  else()
    message(STATUS "cmake-format executable not found")
  endif()
endfunction()

include_directories(include)

file(GLOB SRC_FILES lib/*.c include/*h include/hsfv/*.h)
file(GLOB TEST_FILES tests/*.cpp)
set(target tests)
add_executable(${target} ${TEST_FILES} ${SRC_FILES})
target_link_libraries(${target} PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(${target})

cmake_format(${target})
clang_format(${target})
