#include <stdint.h>
#include <stddef.h>

//#include "stm32l476xx.h"
#include "bootloader_jump.h"
#include "bootloader_validation.h"
#include "test_flash.h"


int main()
{
    //if (bootloader_application_is_valid()) {
     //   bootloader_jump_to_application();
    //}

    volatile status_t test_result = flash_test();

    while (1) {
        // wait for firmware update

    }

    return 0;
}