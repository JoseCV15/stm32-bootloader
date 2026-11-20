#include "bootloader_validation.h"
#include "bootloader_config.h"


static bool is_stack_pointer_valid(uint32_t stack_pointer)
{
    // validate sram1 and sram2
    const bool sram1 = (stack_pointer > SRAM1_START_ADDRESS) && 
                        (stack_pointer <= SRAM1_END_ADDRESS);
    const bool sram2 = (stack_pointer > SRAM2_START_ADDRESS) && 
                        (stack_pointer <= SRAM2_END_ADDRESS);
    const bool algined = ((stack_pointer & (APPLICATION_STACK_ALIGNMENT - 1U)) == 0U);

    return (sram1 || sram2) && algined;
}

static bool is_reset_handler_valid(uint32_t reset_handler)
{
    // Remove thumb bit
    const uint32_t handler_address = reset_handler & ~1UL;
    // Bit_0 of reset handler has to be 1
    const bool thumb_mode = (reset_handler & 1UL) != 0U;
    const bool address_valid = (handler_address >= APPLICATION_START_ADDRESS) && 
                                (handler_address < APPLICATION_END_ADDRESS);
    
    return (thumb_mode && address_valid);
}

bool bootloader_application_is_valid(void)
{
    const volatile uint32_t * const vector_table = (volatile uint32_t*)APPLICATION_START_ADDRESS;
    const uint32_t initial_stack_pointer = vector_table[0];
    const uint32_t reset_handler = vector_table[1];

    if (!is_stack_pointer_valid(initial_stack_pointer) || !is_reset_handler_valid(reset_handler)) {
        return false;
    }
    else {
        return true;
    }

}