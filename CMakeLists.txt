cmake_minimum_required(VERSION 3.14)
project(http_sfv)

set(CMAKE_VERBOSE_MAKEFILE on)

set(CMAKE_C_STANDARD 11)

# Catch2 v3 requires at least C++14
set(CMAKE_CXX_STANDARD 14)

# code coverage with llvm
find_program(LLVM_PROFDATA_EXE NAMES llvm-profdata llvm-profdata-14)
find_program(LLVM_COV_EXE NAMES llvm-cov llvm-cov-14)
set(CODE_COV_FLAGS "-fprofile-instr-generate -fcoverage-mapping")

set(CC_WARNING_FLAGS
    "-Wall -Wno-unused-value -Wno-unused-function -Wno-nullability-completeness -Wno-expansion-to-defined -Werror=implicit-function-declaration -Werror=incompatible-pointer-types"
)
set(CXX_WARNING_FLAGS
    "-Wall -Wno-unused-value -Wno-unused-function -Wno-nullability-completeness -Wno-expansion-to-defined -Werror=implicit-function-declaration -Werror=incompatible-pointer-types -Wno-missing-braces"
)

set(CMAKE_C_FLAGS
    "-g3 -frounding-math ${CODE_COV_FLAGS} ${CC_WARNING_FLAGS} ${CMAKE_C_FLAGS}"
)
set(CMAKE_CXX_FLAGS
    "-g3 ${CODE_COV_FLAGS} ${CXX_WARNING_FLAGS} ${CMAKE_CXX_FLAGS}")

include(FetchContent)
include(ExternalProject)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG 605a34765aa5d5ecbf476b4598a862ada971b0cc # v3.0.1
)
FetchContent_MakeAvailable(Catch2)

# yyjson for test
FetchContent_Declare(
  yyjson_content
  GIT_REPOSITORY https://github.com/ibireme/yyjson.git
  GIT_TAG c3856514de0a67d7b66939bf3ed491a2d6e61277 # 0.5.1
)
FetchContent_GetProperties(yyjson_content)
if(NOT yyjson_content_POPULATED)
  FetchContent_Populate(yyjson_content)
  message(STATUS yyjson_content_SOURCE_DIR=${yyjson_content_SOURCE_DIR})
endif()

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
  message(STATUS libbaseencode_SOURCE_DIR=${libbaseencode_SOURCE_DIR})
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
                    ${yyjson_content_SOURCE_DIR}/src)

set(HttpSfv_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/hsfv.h)
set(HttpSfv_SOURCE_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/allocator.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/base64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/ctype.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/field_value.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/iovec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/bare_item.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/dictionary.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/inner_list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/item.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/parameters.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/skip.c)
add_library(httpsfv STATIC ${HttpSfv_SOURCE_FILES})
set_target_properties(httpsfv PROPERTIES PUBLIC_HEADER ${HttpSfv_HEADER_FILES})
include(GNUInstallDirs)
install(TARGETS httpsfv PUBLIC_HEADER)

set(TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/allocator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/bare_item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/base64.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/ctype.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/dictionary.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/field_value.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/httpwg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/inner_list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/iovec.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/item.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/list.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/parameters.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/skip.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/targeted_cache_control.cpp)

add_executable(
  httpsfv_tests
  ${TEST_FILES} ${libbaseencode_SOURCE_DIR}/src/base32.c
  ${yyjson_content_SOURCE_DIR}/src/yyjson.c ${HttpSfv_SOURCE_FILES})
target_link_libraries(httpsfv_tests PRIVATE Catch2::Catch2WithMain m)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(httpsfv_tests)

add_custom_target(
  check
  LLVM_PROFILE_FILE=httpsfv_tests.profraw ./httpsfv_tests
  COMMAND ${LLVM_PROFDATA_EXE} merge -sparse httpsfv_tests.profraw -o
          httpsfv_tests.profdata
  COMMAND ${LLVM_COV_EXE} show ./httpsfv_tests
          -instr-profile=httpsfv_tests.profdata ${HttpSfv_SOURCE_FILES}
  DEPENDS httpsfv_tests)

clang_format(httpsfv_tests)

target_compile_options(httpsfv_tests PRIVATE -fsanitize=address)
target_link_options(httpsfv_tests PRIVATE -fsanitize=address)
