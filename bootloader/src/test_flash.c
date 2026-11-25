#include "flash.h"
#include "flash_config.h"
#include "test_flash.h"

#include <stdint.h>

#define FLASH_TEST_ADDRESS    APPLICATION_START_ADDRESS
#define FLASH_TEST_PAGE       16U
#define FLASH_TEST_PATTERN    0X123456789ABCDEF0ULL
#define FLASH_TEST_ERASED     0xFFFFFFFFFFFFFFFFULL
#define FLASH_TEST_WRITE_SIZE 8U

status_t flash_test(void) {
    status_t status;
    uint64_t read_value = 0ULL;

    const uint8_t test_data[FLASH_TEST_WRITE_SIZE] = {
        0xF0, 0XDE, 0XBC, 0X9A,
        0X78, 0X56, 0X34, 0X12
    };

    status = flash_erase_page(FLASH_TEST_PAGE);
    if (status != STATUS_OK) {
        return status;
    }

    status = flash_read_dword(FLASH_TEST_ADDRESS, &read_value);
    if (status != STATUS_OK){
        return status;
    }
    if (read_value != FLASH_TEST_ERASED) {
        return STATUS_ERROR;
    }

    // Now write to flash
    status = flash_write(FLASH_TEST_ADDRESS, test_data, FLASH_TEST_WRITE_SIZE);
    if (status != STATUS_OK){
        return status;
    }
    status = flash_read_dword(FLASH_TEST_ADDRESS, &read_value);

    if (read_value != FLASH_TEST_PATTERN) {
        return STATUS_ERROR;
    }

    return STATUS_OK;

}