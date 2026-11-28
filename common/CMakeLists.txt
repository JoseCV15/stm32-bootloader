
add_library(common STATIC
    src/crc.c 
    src/ring_buffer.c 
    src/timer.c
)

target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include

)

target_link_libraries(common PUBLIC
    drivers
    mcu_options
)

target_compile_features(common PUBLIC c_std_11)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(common PRIVATE
        -O0
        -g3
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
    )
endif()
