# Project root directory
get_filename_component(PROJECT_ROOT
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    ABSOLUTE
)

set(COMMON_DIR ${PROJECT_ROOT}/common)
set(DRIVERS_DIR ${PROJECT_ROOT}/drivers)
set(PLATFORM_DIR ${PROJECT_ROOT}/platform/stm32l476)
set(EXTERNAL_DIR ${PROJECT_ROOT}/external)



add_executable(application.elf
    ${PLATFORM_DIR}/startup/application_sttp.c
    #${EXTERNAL_DIR}/cmsis-device-l4/Source/Templates/system_stm32l4xx.c
    ${DRIVERS_DIR}/src/gpio.c
    ${DRIVERS_DIR}/src/rcc_clock.c
    ${DRIVERS_DIR}/src/rcc.c
    ${DRIVERS_DIR}/src/systick.c
    ${DRIVERS_DIR}/src/led.c
    ${DRIVERS_DIR}/src/button.c
    ${DRIVERS_DIR}/src/uart.c
    ${COMMON_DIR}/src/ring_buffer.c
    ${COMMON_DIR}/src/timer.c
    src/main.c
)

# Include directories
target_include_directories(application.elf PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${COMMON_DIR}/include
    ${DRIVERS_DIR}/include
)

# CMSIS
target_link_libraries(application.elf PRIVATE cmsis gcc)

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
    -T${PLATFORM_DIR}/linker/application.ld
)

set_target_properties(application.elf PROPERTIES
    SUFFIX ""
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