#include "packet_parser.h"

#include <string.h>

// Internal helpers
static void packet_parser_clear(packet_parser_t *parser){
    if(!parser) return;
    parser->state = PACKET_PARSER_WAIT_SOF1;
    parser->index = 0;
    parser->header_index = 0U;
    parser->payload_length = 0U;
    parser->payload_index = 0U;
    parser->crc_index = 0U;

    timer_stop(&parser->timeout_timer);
}

static bool packet_parser_store_byte(packet_parser_t *parser, uint8_t byte)
{
    if (!parser || (parser->index >= PACKET_PARSER_BUFFER_SIZE)) {
        return false;
    }

    parser->buffer[parser->index] = byte;
    parser->index++;
    return true;
}

static void packet_parser_resync(packet_parser_t *parser, uint8_t byte)
{
    packet_parser_clear(parser);
    if (byte == PACKET_SOF1) {
        if(packet_parser_store_byte(parser, byte)) {
            parser->state = PACKET_PARSER_WAIT_SOF2;
        }
    }
}

// Public functions
status_t packet_parser_init(packet_parser_t *parser)
{
    status_t status;

    if (!parser) {
        return STATUS_INVALID_ARGUMENT;
    }
   
    memset(parser->buffer, 0, sizeof(parser->buffer));
    parser->state = PACKET_PARSER_WAIT_SOF1;

    status = timer_init(&parser->timeout_timer, PACKET_PARSER_TIMEOUT_MS, TIMER_ONE_SHOT_MODE);
    if (status != STATUS_OK) {
        return status;
    }
    packet_parser_clear(parser);

    return STATUS_OK;
}


void packet_parser_reset(packet_parser_t *parser)
{
    packet_parser_clear(parser);
}

packet_parser_result_t packet_parser_feed(packet_parser_t *parser, uint8_t byte, packet_t *packet)
{
    status_t timer_status;

    if (!parser || !packet) {
        return PACKET_PARSER_ERROR;
    }

    if (parser->state != PACKET_PARSER_WAIT_SOF1) {
        timer_status = timer_reset(&parser->timeout_timer);
        if (timer_status != STATUS_OK) {
            packet_parser_clear(parser);
            return PACKET_PARSER_ERROR;
        }
    }

    switch (parser->state)
    {
    case PACKET_PARSER_WAIT_SOF1:
        if (byte == PACKET_SOF1) {
            if(!packet_parser_store_byte(parser, byte)) {
                packet_parser_clear(parser);
                return PACKET_PARSER_ERROR;
            }
            timer_status =  timer_start(&parser->timeout_timer);
            if (timer_status != STATUS_OK) {
                packet_parser_clear(parser);
                return PACKET_PARSER_ERROR;
            }
            parser->state = PACKET_PARSER_WAIT_SOF2;
        }

        break;

    case PACKET_PARSER_WAIT_SOF2:
        if (byte == PACKET_SOF2) {
            if (!packet_parser_store_byte(parser, byte)) {
                packet_parser_clear(parser);
                return PACKET_PARSER_ERROR;
            }

            parser->state = PACKET_PARSER_RECEIVE_HEADER;
            parser->header_index = 0U;
        }
        else if (byte == PACKET_SOF1) {
            packet_parser_resync(parser, byte);
            timer_status = timer_start(&parser->timeout_timer);
            if (timer_status != STATUS_OK) {
                packet_parser_clear(parser);
                return PACKET_PARSER_ERROR;
            }
        }
        else {
            packet_parser_clear(parser);
        }

        break;
 
    case PACKET_PARSER_RECEIVE_HEADER:
        if (!packet_parser_store_byte(parser, byte)) {
            packet_parser_clear(parser);
            return PACKET_PARSER_ERROR;
        }
        parser->header_index++;
        if (parser->header_index >= (PACKET_HEADER_SIZE - PACKET_SOF_SIZE)) {
            parser->payload_length = (uint16_t)parser->buffer[4] | ((uint16_t)parser->buffer[5] << 8U);
            if (parser->payload_length > PACKET_MAX_PAYLOAD) {
                packet_parser_clear(parser);
                return PACKET_PARSER_ERROR;
            }
            parser->payload_index = 0U;
            if (parser->payload_length == 0U) {
                parser->state = PACKET_PARSER_RECEIVE_CRC;
            }
            else {
                parser->state = PACKET_PARSER_RECEIVE_PAYLOAD;
            }
        }
        break;
    
    /*
    * Receive payload data
    */
    case PACKET_PARSER_RECEIVE_PAYLOAD:
        if (!packet_parser_store_byte(parser, byte)) {
            packet_parser_clear(parser);
            return PACKET_PARSER_ERROR;
        }
        parser->payload_index++;
        if (parser->payload_index >= parser->payload_length) {
            parser->crc_index = 0U;
            parser->state = PACKET_PARSER_RECEIVE_CRC;
        }
        break;

    case PACKET_PARSER_RECEIVE_CRC:
        if (!packet_parser_store_byte(parser, byte)) {
            packet_parser_clear(parser);
            return PACKET_PARSER_ERROR;
        }
        parser->crc_index++;
        if (parser->crc_index >= PACKET_CRC_SIZE) {
            if (packet_decode(parser->buffer, parser->index, packet) == PKT_STATUS_OK) {
                packet_parser_clear(parser);
                return PACKET_PARSER_READY;
            }
            packet_parser_clear(parser);
            return PACKET_PARSER_ERROR;
        }
        break;

    default:
        packet_parser_clear(parser);
        return PACKET_PARSER_ERROR;
    }

    return PACKET_PARSER_NONE;
}

packet_parser_result_t packet_parser_check_timeout(packet_parser_t *parser)
{
    if (!parser) return PACKET_PARSER_ERROR;

    if((parser->state != PACKET_PARSER_WAIT_SOF1) && timer_is_expired(&parser->timeout_timer)) {
        packet_parser_clear(parser);
        return PACKET_PARSER_TIMEOUT;
    }

    return PACKET_PARSER_NONE;
}

