#include <stdint.h>
#include <stddef.h>

//#include "stm32l476xx.h"
#include "bootloader_jump.h"
#include "bootloader_validation.h"


int main()
{
    if (bootloader_application_is_valid()) {
        bootloader_jump_to_application();
    }

    while (1) {
        // wait for firmware update

    }

    return 0;
}