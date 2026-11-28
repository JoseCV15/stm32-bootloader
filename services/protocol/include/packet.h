#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
//#include <stdbool.h>
#include <stddef.h>

/**
 * Packet Header
 * SOF(2 bytes) + SEQ (1 byte) + CMD(1 byte) + LEN (2bytes)
 * Packet Body
 * DATA(0-256 bytes) + CRC (4 bytes)
 * CRC is calculated over SEQ + CMD + LEN + DATA
 */

#define PACKET_SOF1              0xAA
#define PACKET_SOF2              0x55
#define PACKET_SOF_SIZE          2U
#define PACKET_SEQUENCE_SIZE     1U
#define PACKET_COMMAND_SIZE      1U
#define PACKET_LENGTH_SIZE       2U
#define PACKET_HEADER_SIZE       (PACKET_SOF_SIZE + PACKET_SEQUENCE_SIZE + PACKET_COMMAND_SIZE + PACKET_LENGTH_SIZE)
#define PACKET_CRC_SIZE          4U
#define PACKET_MAX_PAYLOAD       256U
#define PACKET_MIN_SIZE          (PACKET_HEADER_SIZE + PACKET_CRC_SIZE)
#define PACKET_MAX_SIZE          (PACKET_MAX_PAYLOAD + PACKET_HEADER_SIZE + PACKET_CRC_SIZE)
#define PACKET_LENGTH_OFFSET     (PACKET_SOF_SIZE + PACKET_SEQUENCE_SIZE + PACKET_COMMAND_SIZE)

typedef enum {
    PKT_STATUS_OK = 0,
    PKT_STATUS_INVALID_ARGUMENT,
    PKT_STATUS_INVALID_LENGTH,
    PKT_STATUS_INVALID_SOF,
    PKT_STATUS_INVALID_CRC,
    PKT_BUFFER_TOO_SMALL
} packet_status_t;

typedef enum {
    PACKET_CMD_SYNC = 0x01,
    PACKET_CMD_GET_INFO,
    PACKET_CMD_ERASE,
    PACKET_CMD_WRITE,
    PACKET_CMD_RESET,
    PACKET_CMD_VERIFY,
    PACKET_CMD_ACK,
    PACKET_CMD_NACK,
    PACKET_CMD_RETRY
} packet_command_t;

typedef struct {
    uint8_t sequence_number;              // identifies the number of packet
    uint8_t command;                      // the type of packet
    uint16_t length;                      // how many bytes exist in the data field
    uint8_t data[PACKET_MAX_PAYLOAD];     // payload
    uint32_t crc;                         // crc 
}packet_t;

/* API */
packet_status_t packet_encode(const packet_t *pkt, uint8_t *buffer, size_t buffer_capacity, size_t *encoded_length);
packet_status_t packet_decode(const uint8_t *buffer, size_t buffer_length, packet_t *pkt);

#endif