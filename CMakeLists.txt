
cmake_minimum_required(VERSION 3.22)

project(stm32_bootloader
    VERSION 1.0
    LANGUAGES C
)

# Project configuration
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# CMSIS dependencies
add_library(cmsis INTERFACE)

target_include_directories(cmsis INTERFACE
    ${PROJECT_SOURCE_DIR}/external/cmsis-core/Include
    ${PROJECT_SOURCE_DIR}/external/cmsis-device-l4/Include
)

target_compile_definitions(cmsis INTERFACE
    STM32L476xx
)

# Firmware targets
add_subdirectory(bootloader)
add_subdirectory(application)

# STM32CubeProgrammer
find_program(STM32_PROGRAMMER_CLI
    NAMES STM32_Programmer_CLI.exe STM32_Programmer_CLI
    PATHS
        "C:/Workspace/STM32CubeProgrammer/bin"
    REQUIRED
)

# Flash application
add_custom_target(flash-app
    COMMAND "${STM32_PROGRAMMER_CLI}"
        -c port=SWD
        -w "${CMAKE_BINARY_DIR}/application/application.hex"
        -v
        -rst
    DEPENDS application.elf
    USES_TERMINAL
    COMMENT "Flashing application layer"
    VERBATIM
)

# Flash bootloader
add_custom_target(flash-btdr
    COMMAND "${STM32_PROGRAMMER_CLI}"
        -c port=SWD
        -w "${CMAKE_BINARY_DIR}/bootloader/bootloader.hex"
        -v
        -rst
    DEPENDS bootloader.elf
    USES_TERMINAL
    COMMENT "Flashing bootloader layer"
    VERBATIM
)

# Flash bootloader + application
add_custom_target(flash
    COMMAND "${STM32_PROGRAMMER_CLI}"
        -c port=SWD
        -w "${CMAKE_BINARY_DIR}/bootloader/bootloader.hex"
        -v
    COMMAND "${STM32_PROGRAMMER_CLI}"
        -c port=SWD
        -w "${CMAKE_BINARY_DIR}/application/application.hex"
        -v
        -rst
    DEPENDS bootloader.elf application.elf
    USES_TERMINAL
    COMMENT "Flashing bootloader and application layers"
    VERBATIM
)


