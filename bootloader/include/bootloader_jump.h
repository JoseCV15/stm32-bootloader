#ifndef BOOTLOADER_JUMP_H
#define BOOTLOADER_JUMP_H

#include <stdbool.h>

/**
 * @brief Transfers execution to the application.
 * 
 * @return false if the application is invalid
 *          Does not return if the jump succeeds.
 */
bool bootloader_jump_to_application(void);

#endif /* BOOTLOADER_JUMP_H */