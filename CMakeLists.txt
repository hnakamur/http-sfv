cmake_minimum_required(VERSION 3.14)
project(http_sfv)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_C_STANDARD 11)
set(CC_WARNING_FLAGS
    "-Wall -Wno-unused-value -Wno-unused-function -Wno-nullability-completeness -Wno-expansion-to-defined -Werror=implicit-function-declaration -Werror=incompatible-pointer-types"
)
set(CMAKE_C_FLAGS "-g3 -frounding-math ${CC_WARNING_FLAGS} ${CMAKE_C_FLAGS}")

# Catch2 v3 requires at least C++14
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
include(ExternalProject)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG 605a34765aa5d5ecbf476b4598a862ada971b0cc # v3.0.1
)
FetchContent_MakeAvailable(Catch2)

# yyjson for test
ExternalProject_Add(
  YyjsonProj
  URL https://github.com/ibireme/yyjson/archive/c3856514de0a67d7b66939bf3ed491a2d6e61277.tar.gz
  URL_HASH
    SHA256=1b406567bbb396c85def5906e3614dec9cd83407e650dc1cb7d21278f0b82ba6
  INSTALL_COMMAND "")
ExternalProject_Get_Property(YyjsonProj SOURCE_DIR BINARY_DIR)
set(YyjsonProj_SOURCE_DIR ${SOURCE_DIR})
set(YyjsonProj_BINARY_DIR ${BINARY_DIR})

# libbaseencode for test
FetchContent_Declare(
  libbaseencode
  GIT_REPOSITORY https://github.com/paolostivanin/libbaseencode.git
  GIT_TAG 3f37e655b7c5a522a5e386cdea50cc19e629ed1a # v1.0.14
)

# Note we don't use FetchContent_MakeAvailable here since we don't want build
# it.
FetchContent_GetProperties(libbaseencode)
if(NOT libbaseencode_POPULATED)
  FetchContent_Populate(libbaseencode)
endif()

# httpwg test cases
ExternalProject_Add(
  HttpwgTests
  URL https://github.com/httpwg/structured-field-tests/archive/faed1f92942abd4fb5d61b1f9f0dc359f499f1d7.tar.gz
  URL_HASH
    SHA256=2bf9b56519cae6cf170eb5c7837f3b744ca2a07c0defd15aa84b541a3cb25a52
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND "")

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
option(RUN_CMAKE_FORMAT
       "If the command cmake-format is avilable, format CMakeFiles.txt." ON)
find_program(CMAKE_FORMAT_EXE cmake-format)
if(RUN_CMAKE_FORMAT)
  if(CMAKE_FORMAT_EXE)
    message(STATUS "Run cmake-format")
    execute_process(COMMAND "${CMAKE_FORMAT_EXE}" -i
                            ${CMAKE_SOURCE_DIR}/CMakeLists.txt)
  else()
    message(STATUS "cmake-format executable not found")
  endif()
endif()

include_directories(include ${libbaseencode_SOURCE_DIR}/src
                    ${YyjsonProj_SOURCE_DIR}/src)

set(HttpSfv_HEADER_FILES include/hsfv.h)
set(HttpSfv_SOURCE_FILES
    lib/allocator.c
    lib/base64.c
    lib/ctype.c
    lib/field_value.c
    lib/iovec.c
    lib/list.c
    lib/bare_item.c
    lib/buffer.c
    lib/dictionary.c
    lib/inner_list.c
    lib/item.c
    lib/parameters.c)
add_library(httpsfv STATIC ${HttpSfv_SOURCE_FILES})
set_target_properties(httpsfv PROPERTIES PUBLIC_HEADER ${HttpSfv_HEADER_FILES})
include(GNUInstallDirs)
install(TARGETS httpsfv PUBLIC_HEADER)

file(GLOB TEST_FILES tests/*.cpp)

set(target httpsfv_tests)
add_executable(${target} ${TEST_FILES} ${libbaseencode_SOURCE_DIR}/src/base32.c)
target_link_libraries(${target} PRIVATE httpsfv Catch2::Catch2WithMain
                                        ${YyjsonProj_BINARY_DIR}/libyyjson.a m)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(${target})

clang_format(${target})

target_compile_options(${target} PRIVATE -fsanitize=address)
target_link_options(${target} PRIVATE -fsanitize=address)
