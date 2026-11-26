#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

uint8_t crc8_calculate(const uint8_t *data, size_t length);
uint32_t crc32_calculate(const uint8_t *data, size_t length);



#endif /* CRC_H */