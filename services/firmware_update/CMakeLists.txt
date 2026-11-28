
add_library(firmware_update STATIC
    src/firmware_update.c
)

target_include_directories(firmware_update PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(firmware_update PUBLIC
    protocol
    drivers
    common
    mcu_options
)

target_compile_features(firmware_update PUBLIC c_std_11)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(firmware_update PRIVATE
        -O0
        -g3
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
    )
endif()
