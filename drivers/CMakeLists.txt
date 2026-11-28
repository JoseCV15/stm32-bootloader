
add_library(drivers STATIC
    src/uart.c
    src/gpio.c
    src/rcc.c
    src/rcc_clock.c
    src/systick.c
    src/led.c
    src/button.c
    src/flash.c
)

target_include_directories(drivers PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/common/include
)

target_link_libraries(drivers PUBLIC
    cmsis
    mcu_options
)

target_compile_features(drivers PUBLIC c_std_11)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(drivers PRIVATE
        -O0
        -g3
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
    )
endif()

