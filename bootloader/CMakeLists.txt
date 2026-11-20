add_executable(bootloader.elf
    ${PROJECT_SOURCE_DIR}/startup/bootloader_sttp.c
    ${PROJECT_SOURCE_DIR}/external/cmsis-device-l4/Source/Templates/system_stm32l4xx.c
    src/main.c
)

# or include specific directory target_include_directories
target_link_libraries(bootloader.elf PRIVATE cmsis)

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

set_target_properties(bootloader.elf PROPERTIES
    SUFFIX ""
)

find_program(CMAKE_OBJCOPY
    NAMES arm-none-eabi-objcopy
    REQUIRED
)

find_program(CMAKE_SIZE
    NAMES arm-none-eabi-size
    REQUIRED
)

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