#ifndef PROTOCOL_MANAGER_H
#define PROTOCOL_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "status.h"
#include "packet.h"
#include "packet_parser.h"

typedef struct {
    status_t (*send)(const uint8_t *buffer, size_t length);
    status_t (*receive)(uint8_t *buffer, size_t buffer_size, size_t *received);
} protocol_transport_t;

typedef enum {
    PROTOCOL_RESULT_OK = 0x00U,
    PROTOCOL_RESULT_UNKNOWN_COMMAND = 0x01U,
    PROTOCOL_RESULT_INVALID_ARGUMENT = 0x02U,
    PROTOCOL_RESULT_FLASH_ERROR = 0x03U,
    PROTOCOL_RESULT_OPERATION_NOT_ALLOWED = 0x04U,
    PROTOCOL_RESULT_NOT_IMPLEMENTED = 0x05U,
    PROTOCOL_RESULT_INVALID_SEQUENCE = 0x06U
} protocol_result_t;


typedef protocol_result_t (*protocol_command_handler_t)(const packet_t *packet,void *context);


typedef struct {
    protocol_transport_t transport;
    protocol_command_handler_t command_handler;
    void *context;
    packet_parser_t parser;
    bool initialized;
    bool last_sequence_valid;
    uint8_t last_sequence;
    packet_t last_request;
    uint8_t last_response[PACKET_MAX_SIZE];
    size_t last_response_length;
} protocol_manager_t;


/* Public API */
status_t protocol_manager_init(protocol_manager_t *manager, const protocol_transport_t *transport,
                               protocol_command_handler_t handler, void *context);
status_t protocol_manager_process(protocol_manager_t *manager);
void protocol_manager_reset_sequence(protocol_manager_t *manager);

#endif // PROTOCOL_MANAGER_H