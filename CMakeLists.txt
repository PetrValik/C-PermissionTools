cmake_minimum_required(VERSION 3.0)
project(hw05)

# -------------------------------------------------------------------
# DO NOT EDIT LINES BELOW lest you break it

# Search for source files.
# If you add a new source to your project, just run ‹cmake -B build -S .›
# or rerun CMake configuration from your IDE.
file(GLOB SOURCES "*.c")

# Test sources
file(GLOB TESTS_GLOB "tests/*.c")
set(SOURCES_LIB ${SOURCES})
list(FILTER SOURCES_LIB EXCLUDE REGEX "/?main\\.c$")

foreach (file_path ${TESTS_GLOB})
    message("Found test: " ${file_path})
endforeach ()

set(TEST_SOURCES
    tests/utils.c
    tests/cut_main_runner.c
    tests/exitus.c
    tests/wmain.c
    ${TESTS_GLOB}
    ${SOURCES_LIB}
)

# Targets
set(EXECUTABLE checkperms)
set(EXECUTABLE_TESTS checkperms-tests)

# Executable
add_executable(${EXECUTABLE} ${SOURCES} main.c)
target_compile_definitions(${EXECUTABLE} PRIVATE _POSIX_C_SOURCE=200809L)

# Tests
add_definitions(-DCUT -DWRAP_INDIRECT)
add_executable(${EXECUTABLE_TESTS} ${TEST_SOURCES})
target_compile_definitions(${EXECUTABLE_TESTS} PRIVATE _POSIX_C_SOURCE=200809L)

# Data files
file(GLOB DATA_FILES "data/*")
foreach (data_file ${DATA_FILES})
    file(COPY ${data_file}
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endforeach()

# Configure compiler warnings
if (CMAKE_C_COMPILER_ID MATCHES Clang OR ${CMAKE_C_COMPILER_ID} STREQUAL GNU)
    # using regular Clang, AppleClang or GCC
    # Strongly suggested: neable -Werror
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wextra -pedantic -g")
endif ()

if (MINGW)
    target_compile_definitions(${EXECUTABLE} PRIVATE __USE_MINGW_ANSI_STDIO=1)
    target_compile_definitions(${EXECUTABLE_TESTS} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()
