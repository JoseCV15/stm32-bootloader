#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>
#include <stdbool.h>
#include "status.h"

/**
 * @brief Check if the address is within range of flash [address, address + size]
 * @return true if success or false if out of bounds
 */
bool flash_is_valid_address(uint32_t address, uint32_t length);

/**
 * @brief Check if address belongs to the application region
 */
bool flash_is_application_address(uint32_t address, uint32_t length);

/**
 * @brief Unlock the flash controller for programming/erasing
 * @return STATUS_OK on success
 */
status_t flash_unlock(void);

/**
 * @brief Lock the flash controller.
 * Should be called after erase or programming operatitons to prevent 
 * unintended modifications
 */
void flash_lock(void);

/**
 * @brief Wait until the flash controller finishes its current operation
 * @return STATUS_OK if it is busy or other status on error / timeout
 */
status_t flash_wait_for_operation(void);


/**
 * @brief Erase on flash page. 
 * Protects the bootloader memory region and locks the controller after successul erasing.
 * IMPORTANT: Only application pages (16 - 511) can be erased.
 * @param page Page Number (0 - 511)
 * @return STATUS_OK on success, or other error codes in case it fail
 */
status_t flash_erase_page(uint32_t page);

/**
 * @brief Program one 64-bit double word into flash
 * @param address - the destination flash address
 * @param data - Pointer to 8 bytes of data
 * - Address must be 8-byte aligned and must belong to the application region
 */
status_t flash_write_dword(uint32_t address, const uint8_t *data);

/**
 * @brief Program a buffer into flash
 * @param address - the destination flash address
 * @param data - Pointer to 8 bytes of data
 * @param length - Number of bytes tot write
 */
status_t flash_write(uint32_t address, const uint8_t *data, uint32_t length);

/**
 * @brief Read bytes from flash into a RAM buffer
 * @param address - the source flash address
 * @param buffer - RAM buffer
 * @param length - Number of bytes to read
 */
status_t flash_read(uint32_t address, uint8_t *buffer, uint32_t length);

/**
 * @brief Read one 64-byte double word from flash
 * @param address - the source flash address
 * @param data - Pointer to data read
 */
status_t flash_read_dword(uint32_t address, uint64_t *data);


#endif