#ifndef PACKET_PARSER_H
#define PACKET_PARSER_H

#include <stdint.h>
#include <stddef.h>

#include "packet.h"
#include "status.h"
#include "timer.h"

#define PACKET_PARSER_BUFFER_SIZE    PACKET_MAX_SIZE
#define PACKET_PARSER_TIMEOUT_MS     3000U

// Packet state machine
typedef enum {
    PACKET_PARSER_WAIT_SOF1 = 0,
    PACKET_PARSER_WAIT_SOF2,
    PACKET_PARSER_RECEIVE_HEADER,
    PACKET_PARSER_RECEIVE_PAYLOAD,
    PACKET_PARSER_RECEIVE_CRC
} packet_parser_state_t;

typedef enum {
    PACKET_PARSER_NONE = 0,
    PACKET_PARSER_READY,
    PACKET_PARSER_ERROR,
    PACKET_PARSER_TIMEOUT
} packet_parser_result_t;

typedef struct {
    packet_parser_state_t state;                 // actual state of state machine
    uint8_t buffer[PACKET_PARSER_BUFFER_SIZE];   // save packet bytes
    size_t index;                                // keep track of the byte store in the buffer
    uint8_t header_index;                        // keep track of header index                        
    uint16_t payload_length;
    uint16_t payload_index;                      // keep track of payload index
    uint8_t crc_index;                           // keep track of crc index
    timer_t timeout_timer;
} packet_parser_t;

status_t packet_parser_init(packet_parser_t *parser);
void packet_parser_reset(packet_parser_t *parser);
packet_parser_result_t packet_parser_feed(packet_parser_t *parser, uint8_t byte, packet_t *packet);
packet_parser_result_t packet_parser_check_timeout(packet_parser_t *parser);





#endif