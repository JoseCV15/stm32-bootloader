add_executable(application.elf
    ${PROJECT_SOURCE_DIR}/startup/application_sttp.c
    ${PROJECT_SOURCE_DIR}/external/cmsis-device-l4/Source/Templates/system_stm32l4xx.c
    src/main.c
)

target_link_libraries(application.elf PRIVATE cmsis)

target_compile_options(application.elf PRIVATE
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

target_link_options(application.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -nostdlib
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/application.map
    -T${PROJECT_SOURCE_DIR}/linker/application.ld
)

set_target_properties(application.elf PROPERTIES
    SUFFIX ""
)


find_program(CMAKE_SIZE
    NAMES arm-none-eabi-size
    REQUIRED
)

add_custom_command(TARGET application.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY}
        -O ihex
        $<TARGET_FILE:application.elf>
        $<TARGET_FILE_DIR:application.elf>/application.hex
    COMMAND ${CMAKE_OBJCOPY}
            -O binary
            $<TARGET_FILE:application.elf>
            $<TARGET_FILE_DIR:application.elf>/application.bin
    COMMAND ${CMAKE_SIZE}
            $<TARGET_FILE:application.elf>
    COMMENT "Generating firmware artifacts for application.elf"
    VERBATIM
)