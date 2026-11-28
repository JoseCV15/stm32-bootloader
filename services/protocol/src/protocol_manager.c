#include "protocol_manager.h"

#include <string.h>

#define PROTOCOL_RX_BUFFER_SIZE 64U
#define PROTOCOL_CMD_ACK  PACKET_CMD_ACK
#define PROTOCOL_CMD_NACK PACKET_CMD_NACK

/* Compare two already validated requests */
static bool protocol_packet_equal(const packet_t *a, const packet_t *b)
{
    if ((a == NULL) || (b == NULL))
    {
        return false;
    }

    if ((a->sequence_number != b->sequence_number) || (a->command != b->command) || (a->length != b->length)) {
        return false;
    }

    if (a->length == 0U) {
        return true;
    }

    return (memcmp(a->data, b->data, a->length) == 0);
}

static status_t protocol_fill_packet(packet_t *packet, uint8_t sequence, protocol_result_t result)
{
    if (!packet) return STATUS_INVALID_ARGUMENT;

    packet->sequence_number = sequence;
    packet->command = (result == PROTOCOL_RESULT_OK) ? PACKET_CMD_ACK : PACKET_CMD_NACK;
    packet->length = 1U;
    packet->data[0] = (uint8_t)result;

    return STATUS_OK;
}

/* Encode and transmit an ACK or NACK */
static status_t protocol_send_response(protocol_manager_t *manager, uint8_t sequence, protocol_result_t result, bool cache_response)
{
    packet_t response = {0};

    uint8_t encoded[PACKET_MAX_SIZE];
    size_t encoded_length = 0U;

    if (protocol_fill_packet(&response, sequence, result) != STATUS_OK) {
        return STATUS_ERROR;
    }

    if (packet_encode(&response, encoded, sizeof(encoded), &encoded_length) != PKT_STATUS_OK){
        return STATUS_ERROR;
    }

    /*
     * Cache the response before transmission.
     * If transmission fails, a duplicate request
     * can trigger another transmission.
     */
    if (cache_response) {
        memcpy(manager->last_response, encoded, encoded_length);
        manager->last_response_length = encoded_length;
    }

    return manager->transport.send(encoded, encoded_length);
}


/* Resend the cached response */
static status_t protocol_resend_last_response(protocol_manager_t *manager)
{
    if (manager->last_response_length == 0U) {
        return STATUS_ERROR;
    }

    return manager->transport.send(manager->last_response, manager->last_response_length);
}


/* Process a complete packet */
static status_t protocol_handle_packet(protocol_manager_t *manager, const packet_t *packet)
{
    protocol_result_t result;

    if ((manager == NULL) || (packet == NULL)) {
        return STATUS_INVALID_ARGUMENT;
    }

    /*
     * Case 1: Same sequence as the previous request.
     */
    if (manager->last_sequence_valid && (packet->sequence_number == manager->last_sequence))
    {
        /*
         * It is a real duplicate only if its contents
         * are identical to the previous request.
         */
        if (protocol_packet_equal(packet, &manager->last_request)) {
            return protocol_resend_last_response(manager);
        }

        /*
         * Same sequence, different request.
         */
        return protocol_send_response(manager, packet->sequence_number,
                    PROTOCOL_RESULT_INVALID_SEQUENCE,false);
    }

    /*
     * Case 2: Check the expected sequence.
     */
    if (manager->last_sequence_valid) {
        uint8_t expected_sequence = (uint8_t)(manager->last_sequence + 1U);

        if (packet->sequence_number != expected_sequence) {
            return protocol_send_response(manager,packet->sequence_number,
                PROTOCOL_RESULT_INVALID_SEQUENCE,false);
        }
    }

    /*
     * Case 3: Execute the new request.
     */
    result = manager->command_handler(packet,manager->context);

    manager->last_request = *packet;
    manager->last_sequence = packet->sequence_number;
    manager->last_sequence_valid = true;

    /*
     * Case 4: Respond to the sender.
     */
    return protocol_send_response(manager, packet->sequence_number,  result, true);
}


/* Initialize Protocol Manager */
status_t protocol_manager_init(protocol_manager_t *manager, const protocol_transport_t *transport,
    protocol_command_handler_t handler, void *context)
{
    status_t status;

    if ((manager == NULL) || (transport == NULL) || (transport->send == NULL) ||
        (transport->receive == NULL) || (handler == NULL)) {
        return STATUS_INVALID_ARGUMENT;
    }

    memset(manager, 0, sizeof(*manager));

    manager->transport = *transport;
    manager->command_handler = handler;
    manager->context = context;

    status = packet_parser_init(&manager->parser);

    if (status != STATUS_OK) {
        return status;
    }

    manager->initialized = true;

    return STATUS_OK;
}


/* Process incoming transport bytes */
status_t protocol_manager_process(protocol_manager_t *manager)
{
    uint8_t rx_buffer[PROTOCOL_RX_BUFFER_SIZE];
    size_t received = 0U;

    if ((manager == NULL) || !manager->initialized) {
        return STATUS_INVALID_ARGUMENT;
    }

    status_t status = manager->transport.receive(rx_buffer, sizeof(rx_buffer), &received);
    if (status != STATUS_OK) {
        return status;
    }

    if (received > sizeof(rx_buffer)){
        return STATUS_ERROR;
    }

    for (size_t i = 0U; i < received; i++) {
        packet_t packet;

        /*
         * Expire an incomplete packet before
         * processing the next received byte.
         */
        (void)packet_parser_check_timeout(&manager->parser);
        packet_parser_result_t result = packet_parser_feed(&manager->parser, rx_buffer[i], &packet);

        switch (result)
        {
            case PACKET_PARSER_READY: {
                status = protocol_handle_packet(manager, &packet);
                if (status != STATUS_OK) {
                    return status;
                }
                break;
            }

            case PACKET_PARSER_ERROR:
            case PACKET_PARSER_TIMEOUT:
            case PACKET_PARSER_NONE:
            default: {
                break;
            }
        }
    }

    if (received == 0U) {
        (void)packet_parser_check_timeout(&manager->parser);
    }

    return STATUS_OK;
}

void protocol_manager_reset_sequence(protocol_manager_t *manager)
{
    if (manager == NULL) {
        return;
    }

    manager->last_sequence_valid = false;
    manager->last_sequence = 0U;
    manager->last_response_length = 0U;

    memset(&manager->last_request, 0, sizeof(manager->last_request));
}
