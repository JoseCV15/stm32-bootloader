
cmake_minimum_required(VERSION 3.20)

project(packet_tests C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Project root directory
get_filename_component(PROJECT_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    ABSOLUTE
)

add_executable(test_packet
    test_packet.c
    ${PROJECT_ROOT}/common/src/packet.c
    ${PROJECT_ROOT}/common/src/crc.c

)

target_include_directories(test_packet PRIVATE
    ${PROJECT_ROOT}/common/include
)
if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_packet PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -g
    )
endif()

enable_testing()
add_test(NAME packet_tests COMMAND test_packet)