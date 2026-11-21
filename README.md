# STM32 custom bootloader

## Overview

I build this project to deep dive into understanding bootloaders for the STM32L476 family using only C, CMSIS and low level register access.
The project is structured into independent layers in order to improve modularity, reusability and maintainability.

## Software Architecture
The project is divided into four main layers.
1. **Hardware Drivers** - low-level access to microcontroller peripherals.
2. **Common /Middleware** - reusable software components
3. **Services** - higher-level features built on top of lower layers.
4. **Bootloader / Application** - product-specific logic.


## Project Structure

```text
stm32-bootloader/
│
├── external/       # CMSIS submodules
├── platform/       # Startup, linker scripts and system configuration
├── drivers/        # Hardware drivers (GPIO, UART, FLASH, RCC, etc.)
├── common/         # Reusable components (Ring Buffer, CRC32, etc.)
├── services/       # Higher-level services (Firmware Update, Communication)
├── bootloader/     # Bootloader logic
├── application/    # Application firmware
├── tests/          # Unit tests
├── tools/          # Firmware updater
└── CMakeLists.txt
```


## Architecture rules

- Drivers must not depend on application or bootloader
- Common components must be hardware-independent.
- Services can use drivers and common components
- Bootloader and application can share drivers, common components and services. However, they are built 
as separate executables with separate linker scripts.


## Build System

The project uses CMake. 
The bootloader and application are compiled separately, each with its own startup configuration and linker scripts.
