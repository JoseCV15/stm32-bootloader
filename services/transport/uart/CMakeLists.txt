
add_library(uart_transport STATIC
    src/uart_transport.c
)

target_include_directories(uart_transport PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(uart_transport PUBLIC
    drivers
    common
    mcu_options
)

target_compile_features(uart_transport PUBLIC c_std_11)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(uart_transport PRIVATE
        -O0
        -g3
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
    )
endif()
