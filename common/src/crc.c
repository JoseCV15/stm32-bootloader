#include "crc.h"


#define CRC32_POLYNOMIAL      0xEDB88320UL
#define CRC32_INITIAL_VALUE   0xFFFFFFFFUL
#define CRC32_FINAL_XOR       0xFFFFFFFFUL

uint8_t crc8_calculate(const uint8_t *data, size_t length)
{
    uint8_t crc = 0x00U;
    uint8_t polynomial = 0x07U;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8U; j++) {
            if ((crc & 0x80U)!= 0U) {
                crc = (uint8_t)((crc << 1U) ^ polynomial);
            }
            else {
                crc = (uint8_t)(crc << 1U);
            }
        }
    }

    return crc;
}

uint32_t crc32_calculate(const uint8_t *data, size_t length)
{
    uint32_t crc = CRC32_INITIAL_VALUE;

    if (!data && (length > 0U)) {
        return 0U;
    }

    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8U; j++)
        {
            if ((crc & 1U) != 0U) {
                crc = (crc >> 1U) ^ CRC32_POLYNOMIAL;
            }
            else {
                crc >>= 1U;
            }
        }
    }

    return crc ^ CRC32_FINAL_XOR;
}
