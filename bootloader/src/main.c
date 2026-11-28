#include <stdint.h>
#include <stddef.h>

//#include "stm32l476xx.h"
#include "bootloader_jump.h"
#include "bootloader_validation.h"
#include "uart.h"
#include "uart_transport.h"
#include "firmware_update.h"

volatile uint32_t debug_stage = 0U;
volatile status_t debug_status = STATUS_OK;

int main()
{
    //if (bootloader_application_is_valid()) {
    //   bootloader_jump_to_application();
    //}

    debug_stage = 1U;
    debug_status = uart_init();
    if (debug_status != STATUS_OK) {
        debug_stage = 2U;
        while(1){}
    }

    protocol_transport_t transport = {
        .send = uart_transport_send,
        .receive = uart_transport_receive
    };

    debug_stage = 3U;

    debug_status = firmware_update_init(&transport);
    if(debug_status != STATUS_OK) {
        debug_stage = 4U;
        while(1){}
    }

    debug_stage = 5U;

    
    while (1) {
        debug_status = firmware_update_process();
        if (debug_status != STATUS_OK) {
            debug_stage = 6U;
        }
    }
   

    return 0;
}