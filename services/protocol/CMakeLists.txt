
add_library(protocol STATIC
    src/packet.c 
    src/packet_parser.c
    src/protocol_manager.c
)

target_include_directories(protocol PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(protocol PUBLIC
    common
    mcu_options
)

target_compile_features(protocol PUBLIC c_std_11)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(protocol PRIVATE
        -O0
        -g3
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
    )
endif()
