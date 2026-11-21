
cmake_minimum_required(VERSION 3.20)

project(ring_buffer_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Project root directory
get_filename_component(PROJECT_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    ABSOLUTE
)

add_executable(test_ring_buffer
    test_ring_buffer.c
    ${PROJECT_ROOT}/common/src/ring_buffer.c
)

target_include_directories(test_ring_buffer PRIVATE
    ${PROJECT_ROOT}/common/include
)

target_compile_options(test_ring_buffer PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -g
)
