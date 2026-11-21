#include "crc.h"


#define CRC32_POLYNOMIAL      0xEDB88320UL
#define CRC32_INITIAL_VALUE   0xFFFFFFFFUL
#define CRC32_FINAL_XOR       0xFFFFFFFFUL

uint8_t crc8_calculate(const uint8_t *data, uint32_t length)
{
    uint8_t crc = 0x00;
    uint8_t polynomial = 0x07;
    for (uint32_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial;
            }
            else {
                crc <<= 1;
            }
        }
    }

    return crc;
}

uint8_t crc32_calculate(const uint8_t *data, uint32_t length)
{
    uint32_t crc = CRC32_INITIAL_VALUE;

    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1) {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            }
            else {
                crc >>= 1;
            }
        }
    }
    return (~crc) & CRC32_FINAL_XOR;
}
