#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

uint8_t crc8_calculate(const uint8_t *data, uint32_t length);
uint8_t crc32_calculate(const uint8_t *data, uint32_t length);



#endif /* CRC_H */