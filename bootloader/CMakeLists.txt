
set(COMMON_DIR ${PROJECT_SOURCE_DIR}/common)

add_executable(bootloader.elf
    ${PROJECT_SOURCE_DIR}/startup/bootloader_sttp.c
    ${PROJECT_SOURCE_DIR}/external/cmsis-device-l4/Source/Templates/system_stm32l4xx.c
    src/main.c
    src/bootloader_validation.c
    src/bootloader_jump.c
    ${COMMON_DIR}/src/crc.c
    ${COMMON_DIR}/src/ring_buffer.c
)

# Include directories
target_include_directories(bootloader.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${COMMON_DIR}/include
)

# CMSIS
target_link_libraries(bootloader.elf PRIVATE 
    cmsis
)

# Compiler options
target_compile_options(bootloader.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -ffreestanding
    -fno-builtin
    -ffunction-sections
    -fdata-sections
    -Wall
    -Wextra
    -Wpedantic

    -O0
    -g3
)

# Linker options
target_link_options(bootloader.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -nostdlib
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/bootloader.map
    -T${PROJECT_SOURCE_DIR}/linker/bootloader.ld
)

# Target properties
set_target_properties(bootloader.elf PROPERTIES
    SUFFIX ""
)


# Firmware artifactt tools

find_program(CMAKE_OBJCOPY
    NAMES arm-none-eabi-objcopy
    REQUIRED
)

find_program(CMAKE_SIZE
    NAMES arm-none-eabi-size
    REQUIRED
)

# Generate HEX, BIN, and display firmware size
add_custom_command(TARGET bootloader.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        -O ihex
        $<TARGET_FILE:bootloader.elf>
        $<TARGET_FILE_DIR:bootloader.elf>/bootloader.hex
    COMMAND ${CMAKE_OBJCOPY}
            -O binary
            $<TARGET_FILE:bootloader.elf>
            $<TARGET_FILE_DIR:bootloader.elf>/bootloader.bin
    COMMAND ${CMAKE_SIZE}
            $<TARGET_FILE:bootloader.elf>
    COMMENT "Generating firmware artifacts for bootloader.elf"
    VERBATIM
)