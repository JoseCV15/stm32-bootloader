
# Project root directory
get_filename_component(PROJECT_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    ABSOLUTE
)

set(PLATFORM_DIR ${PROJECT_ROOT}/platform/stm32l476)

add_executable(bootloader.elf
    ${PLATFORM_DIR}/startup/bootloader_sttp.c
    src/main.c
    src/bootloader_validation.c
    src/bootloader_jump.c
)

# Include directories
target_include_directories(bootloader.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)


target_link_libraries(bootloader.elf PRIVATE 
    cmsis
    common
    drivers
    firmware_update
    protocol
    uart_transport
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
    #-nostdlib
    -nostartfiles
    --specs=nano.specs
    --specs=nosys.specs

    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/bootloader.map
    -T${PLATFORM_DIR}/linker/bootloader.ld
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