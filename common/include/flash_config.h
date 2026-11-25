#ifndef FLASH_CONFIG_H
#define FLASH_CONFIG_H

#include <stdint.h>
#include "stm32l476xx.h"

/*
* STM32L476RG - Flash organization
* Flash size: 1 MB
* Flash base: 0x08000000
* Flash end:  0x08100000 (excl)
*
* Bootloader region
*  0x08000000 - 0x08007FFF
*
* Application region
*  0x08008000 - 0x080FFFFF
*/

#define FLASH_BASE_ADDRESS            0x08000000UL
#define FLASH_PAGE_SIZE               2048UL
#define FLASH_PAGE_COUNT              512UL
#define FLASH_END_ADDRESS             (FLASH_BASE_ADDRESS + FLASH_SIZE)

/* Bootloader configuration */
#define BOOTLOADER_START_ADDRESS      FLASH_BASE_ADDRESS
#define BOOTLOADER_SIZE               (32UL * 1024UL)
#define BOOTLOADER_END_ADDRESS        (BOOTLOADER_START_ADDRESS + BOOTLOADER_SIZE)

/* Application configuration */
#define APPLICATION_START_ADDRESS      (BOOTLOADER_END_ADDRESS)
#define APPLICATION_SIZE               (FLASH_SIZE - BOOTLOADER_SIZE)
//#define APPLICATION_MAX_SIZE           (FLASH_END_ADDRESS - BOOTLOADER_END_ADDRESS)
#define APPLICATION_END_ADDRESS        (FLASH_END_ADDRESS)

#define BOOTLOADER_FIRST_PAGE          0UL
#define BOOTLOADER_LAST_PAGE           15UL
#define APPLICATION_FIRST_PAGE         16UL
#define APPLICATION LAST_PAGE          511UL

/* Vector Table */
#define APPLICATION_VT_OFFSET          (BOOTLOADER_SIZE)

/* SRAM Configuration 
*  SRAM1 size: 96 kB
*  SRAM2 size: 32 kB
*/
#define SRAM1_START_ADDRESS             0x20000000UL
#define SRAM1_SIZE                      0x00018000UL
#define SRAM1_END_ADDRESS               (SRAM1_START_ADDRESS + SRAM1_SIZE)
#define SRAM2_START_ADDRESS             0x10000000UL
#define SRAM2_END_ADDRESS               (SRAM2_START_ADDRESS + SRAM2_SIZE)

/* Firmware validation */
#define APPLICATION_VECTOR_SIZE        8U
#define APPLICATION_STACK_ALIGNMENT    8U

#endif /* BOOTLOADER_CONFIG_H */