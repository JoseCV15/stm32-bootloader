#include "flash.h"
#include "flash_config.h"
#include "systick.h"
#include "stm32l476xx.h"
#include <stdbool.h>
#include <string.h>

#define ALIGNED(address, byte)       (((address) + ((byte) - 1)) & ~((byte) - 1))
#define IS_ALIGNED(address, byte)    (((address) % (byte)) == 0U)

#define FLASH_KEY1         0x45670123
#define FLASH_KEY2         0xCDEF89AB

#define FLASH_TIMEOUT_MS   500U
#define FLASH_WRITE_SIZE   8U

#define FLASH_ERROR_FLAGS (\
    FLASH_SR_OPERR | FLASH_SR_PROGERR | FLASH_SR_WRPERR | FLASH_SR_PGAERR | \
    FLASH_SR_SIZERR | FLASH_SR_PGSERR)

#define FLASH_CLEAR_FLAGS      (FLASH_ERROR_FLAGS | FLASH_SR_EOP)

// clear flash status flags
static void flash_clear_status_flags(void) { FLASH->SR = FLASH_CLEAR_FLAGS;}
// Check if the flash controller reported an error
static bool flash_check_errors(void) { return ((FLASH->SR & FLASH_ERROR_FLAGS) != 0U);}
// Check if 8-byte flash dword is reased (0xFFFFFFFFFFFFFFFF)
static bool flash_is_dword_erased(uint32_t address)
{
    volatile const uint32_t *flash_address = (volatile const uint32_t *)address;
    uint32_t first_word = *flash_address;
    uint32_t second_word = *(flash_address + 1 );

    if ((first_word == 0xFFFFFFFFUL) && (second_word == 0xFFFFFFFFUL)) {
        return true;
    }
    else {
        return false;
    }
}

/**
 * Program one dword
 * The flash controller must already be unlocked
 * The address must be 8-byte aligned
 */
static status_t flash_program_dword(uint32_t address, const uint8_t *data)
{
    uint32_t lword, hword;
    status_t status;

    volatile uint32_t *flash_address = (volatile uint32_t *)address;
    if (!flash_is_dword_erased(address)) {
        return STATUS_ERROR;
    }
    // Copy data  void *memcpy(void *dest, const void *src, size_t n);
    memcpy(&lword, data, sizeof(lword));
    memcpy(&hword, data + 4U, sizeof(hword));

    flash_clear_status_flags();

    // Enable programming mode
    FLASH->CR |= FLASH_CR_PG;

    *flash_address = lword;
    *(flash_address + 1U) = hword;

    status = flash_wait_for_operation();
    if (status != STATUS_OK) {
        return status;
    }

    // Disable programming mode
    FLASH->CR &= ~FLASH_CR_PG;

    if (flash_check_errors()) {
        flash_clear_status_flags();
        return STATUS_ERROR;
    }
    flash_clear_status_flags();

    return STATUS_OK;
}


bool flash_is_valid_address(uint32_t address, uint32_t length)
{
    uint64_t end_address = (uint64_t)address + (uint64_t)length;
    if ((length == 0U)) {
        return false;
    }
    if (address < FLASH_BASE_ADDRESS) {
        return false;
    }
    if (end_address > (uint64_t)FLASH_END_ADDRESS) {
        return false;
    }
    return true;
}

bool flash_is_application_address(uint32_t address, uint32_t length)
{
    uint64_t end_address = (uint64_t)address + (uint64_t)length;
    if ((length == 0U)) {
        return false;
    }
    if (address < APPLICATION_START_ADDRESS) {
        return false;
    }
    if (end_address > (uint64_t)APPLICATION_END_ADDRESS) {
        return false;
    }
    return true;
}

status_t flash_unlock(void)
{
    if ((FLASH->CR & FLASH_CR_LOCK) != 0U) {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }

    if ((FLASH->CR & FLASH_CR_LOCK) != 0U) {
        return STATUS_BUSY;
    }
    return STATUS_OK;
}

void flash_lock(void) 
{ 
    FLASH->CR |= FLASH_CR_LOCK;
}

status_t flash_wait_for_operation(void)
{
    uint32_t start_tick = systick_get_tick();

    while ((FLASH->SR & FLASH_SR_BSY) != 0U) {
        if ((uint32_t)(systick_get_tick() - start_tick)  >= FLASH_TIMEOUT_MS) {
            return STATUS_TIMEOUT;
        }
    }
    return STATUS_OK;
}

status_t flash_erase_page(uint32_t page)
{
    status_t status;
    uint32_t page_in_bank;

    // Validate page number and protect bootloader region
    if ((page >= FLASH_PAGE_COUNT) || (page < APPLICATION_FIRST_PAGE)){
        return STATUS_INVALID_ARGUMENT;
    }

    // Wait for any previous operation
    status = flash_wait_for_operation();
    if (status != STATUS_OK) {
        return status;
    }

    // Unlock the controller
    status = flash_unlock();
    if (status != STATUS_OK) {
        return status;
    }

    // Clear errors from previous operation
    flash_clear_status_flags();

    page_in_bank = page % 256U;

    FLASH->CR &= ~(FLASH_CR_PNB | FLASH_CR_BKER |
                    FLASH_CR_PER | FLASH_CR_PG);

    // Enable page erase mode
    FLASH->CR |= FLASH_CR_PER;

    // Select page inside the bank
    FLASH->CR |= (page_in_bank << FLASH_CR_PNB_Pos);
    // Select Bank 2 (256 -511)
    if (page >= 256U) {
        FLASH->CR |= FLASH_CR_BKER;
    }

    // Start the erase and wait for operation
    FLASH->CR |= FLASH_CR_STRT;
    status = flash_wait_for_operation();
    if (status != STATUS_OK) {
        return status;
    }

    if (flash_check_errors()) {
        status = STATUS_ERROR;
    }
    // Disable page erase mode
    FLASH->CR &= ~FLASH_CR_PER;
    
    flash_clear_status_flags();
    flash_lock();

    return status;
}


status_t flash_write_dword(uint32_t address, const uint8_t *data)
{
    status_t status;

    if (!data) {
        return STATUS_INVALID_ARGUMENT;
    }

    // 8-byte alginment
    if (!IS_ALIGNED(address, 8U)) {
        return STATUS_NOT_SUPPORTED;
    }

    if(!flash_is_application_address(address, 8)) {
        return STATUS_NOT_SUPPORTED;
    }

    // Wait for any previous operation
    status = flash_wait_for_operation();
    if (status != STATUS_OK) {
        return status;
    }

    // Unlock the controller
    status = flash_unlock();
    if (status != STATUS_OK) {
        return status;
    }

    status = flash_program_dword(address, data);

    if ((status == STATUS_BUSY) && (FLASH->SR & FLASH_SR_BSY) != 0U) {
        return status;
    }

    FLASH->CR &= ~FLASH_CR_PG;

    flash_lock();

    return status;
}


status_t flash_write(uint32_t address, const uint8_t *data, uint32_t length)
{
    status_t status;
    // Buffer to save data in RAM before writing to flash
    uint8_t write_buffer[FLASH_WRITE_SIZE];
    uint32_t offset = 0U;

    if (!data || (length == 0U)) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!IS_ALIGNED(address, 8U)) {
        return STATUS_NOT_SUPPORTED;
    }

    // Number of bytes actually programmed
    uint64_t padded_length = IS_ALIGNED((uint64_t)length, 8ULL);
    if (padded_length > UINT32_MAX || !flash_is_application_address(address, (uint32_t)padded_length)) {
        return STATUS_INVALID_ARGUMENT;
    }
    status = flash_wait_for_operation();
    if (status != STATUS_OK) {
        return status;
    }

    status = flash_unlock();
    if (status != STATUS_OK) {
        return status;
    }

    while (offset < length) {
        uint32_t remaining = length - offset;

        memset(write_buffer, 0xFF, FLASH_WRITE_SIZE);
        uint32_t bytes_to_cpy = (remaining >= 8U) ? 8U : remaining;
        memcpy(write_buffer, data + offset, bytes_to_cpy);

        status = flash_program_dword(address + offset, write_buffer);
        if ((status == STATUS_BUSY) && (FLASH->SR & FLASH_SR_BSY) != 0U) {
            return status;
        }

        if (status != STATUS_OK) {
            break;
        }

        offset += 8U;
    }

    // Disable programming
    FLASH->CR &= ~FLASH_CR_PG;

    flash_lock();

    return status;
}

status_t flash_read(uint32_t address, uint8_t *buffer, uint32_t length)
{
    if (!buffer || !flash_is_valid_address(address, length)) {
        return STATUS_INVALID_ARGUMENT;
    }

    volatile const uint8_t *flash_source = (volatile const uint8_t*)address;
    for (uint32_t i = 0; i < length; i++)
    {
        buffer[i] = flash_source[i];
    }

    return STATUS_OK;
}

status_t flash_read_dword(uint32_t address, uint64_t *data)
{
    if (!data) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!IS_ALIGNED(address, 8U)) {
        return STATUS_NOT_SUPPORTED;
    }

    if (!flash_is_valid_address(address, 8U)) {
        return STATUS_INVALID_ARGUMENT;
    }

    volatile const uint32_t *flash_source = (volatile const uint32_t*)address;
    uint32_t lword = *flash_source;
    uint32_t hword = *(flash_source + 1U);
    *data = ((uint64_t)hword << 32U) | lword;

    return STATUS_OK;

}