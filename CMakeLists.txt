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


