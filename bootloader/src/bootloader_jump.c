#include <stdint.h>
#include <stdbool.h>

#include "stm32l476xx.h"
#include "bootloader_config.h"
#include "bootloader_validation.h"
#include "bootloader_jump.h"

typedef void (*application_entry_t)(void);

bool bootloader_jump_to_application(void)
{
    if (!bootloader_application_is_valid()) {
        return false;
    }

    const volatile uint32_t * const vector_table = (volatile uint32_t*)APPLICATION_START_ADDRESS;
    const uint32_t application_msp = vector_table[0];
    const uint32_t application_reset_handler = vector_table[1];

    const application_entry_t application_entry = (application_entry_t)application_reset_handler;
    
    // Disable interrupts during handover
    __disable_irq();
    // Stop systick
    SysTick->CTRL = 0U;
    SysTick->LOAD = 0U;
    SysTick->VAL = 0U;

    // Disable and clear NVIC
    for (uint32_t i = 0U; i < 3U; i++) {
        NVIC->ICER[i] = 0xFFFFFFFFUL;
        NVIC->ICPR[i] = 0xFFFFFFFFUL;
    }

    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;

    SCB->VTOR = APPLICATION_START_ADDRESS;

    __DSB();
    __ISB();

    // Privileged thread mode and msp
    __set_CONTROL(0U);
    __set_BASEPRI(0U);
    __set_FAULTMASK(0U);

    __DSB();
    __ISB();

    __set_MSP(application_msp);

    application_entry();

    __builtin_unreachable();

}