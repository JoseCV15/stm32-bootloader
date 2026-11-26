#include "packet.h"
#include "crc.h"

#include <string.h>

/* Private helpers */
// Convert 16-bit value to little-endian bytes
static void packet_write_u16_le(uint8_t *buffer, uint16_t value)
{
    buffer[0] = (uint8_t)(value & 0xFFU);
    buffer[1] = (uint8_t)((value >> 8U) & 0xFFU);
}

// Read 16-byte little endian value
static uint16_t packet_read_u16_le(const uint8_t *buffer)
{
    return ((uint16_t)buffer[0] | (uint16_t)(buffer[1] << 8U));
}

// Convert 32-byte value to little endian
static void packet_write_u32_le(uint8_t *buffer, uint32_t value)
{
    buffer[0] = (uint8_t)(value & 0xFFU);
    buffer[1] = (uint8_t)((value >> 8U) & 0xFFU);
    buffer[2] = (uint8_t)((value >> 16U) & 0xFFU);
    buffer[3] = (uint8_t)((value >> 24U) & 0xFFU);
}

// Read 32-byte little endian value
static uint32_t packet_read_u32_le(const uint8_t *buffer)
{
    return (((uint32_t)buffer[0]) | ((uint32_t)buffer[1] << 8U) |
            ((uint32_t)buffer[2] << 16U) | ((uint32_t)buffer[3] << 24U));
}

/* Public API */
packet_status_t packet_encode(const packet_t *pkt, uint8_t *buffer, size_t buffer_capacity,
                               size_t *encoded_length)
{
    uint32_t crc;
    size_t total_length, crc_offset, crc_length;

    if (!pkt || !buffer || !encoded_length) {
        return PKT_STATUS_INVALID_ARGUMENT;
    }
    *encoded_length = 0U;

    if (pkt->length > PACKET_MAX_PAYLOAD) {
        return PKT_STATUS_INVALID_LENGTH;
    }

    total_length = PACKET_HEADER_SIZE + (size_t)pkt->length + PACKET_CRC_SIZE;
    crc_offset = PACKET_HEADER_SIZE + (size_t)pkt->length;
    crc_length = PACKET_SEQUENCE_SIZE + PACKET_COMMAND_SIZE + PACKET_LENGTH_SIZE + (size_t)pkt->length;

    if (buffer_capacity < total_length) {
        return PKT_BUFFER_TOO_SMALL;
    }

    buffer[0] = PACKET_SOF1;
    buffer[1] = PACKET_SOF2;
    buffer[2] = pkt->sequence_number;
    buffer[3] = pkt->command;
    packet_write_u16_le(&buffer[PACKET_LENGTH_OFFSET], pkt->length);
    if (pkt->length > 0U) {
        memcpy(&buffer[PACKET_HEADER_SIZE], pkt->data, pkt->length);
    }

    // SEQ + CMD + LEN + DATA
    crc = crc32_calculate(&buffer[PACKET_SOF_SIZE], crc_length);
    packet_write_u32_le(&buffer[crc_offset], crc);

    *encoded_length = total_length;
    return PKT_STATUS_OK;
}

packet_status_t packet_decode(const uint8_t *buffer, size_t buffer_length, packet_t *pkt)
{
    uint32_t received_crc, calculated_crc;
    size_t expected_length, crc_length, payload_length, crc_offset;

    if (!buffer || !pkt) {
        return PKT_STATUS_INVALID_ARGUMENT;
    }
    if ((buffer_length < PACKET_MIN_SIZE) || (buffer_length > PACKET_MAX_SIZE)) {
        return PKT_STATUS_INVALID_LENGTH;
    }

    // Validate soft
    if ((buffer[0] != PACKET_SOF1) || (buffer[1] != PACKET_SOF2)) {
        return PKT_STATUS_INVALID_SOF;
    }

    // 
    payload_length = packet_read_u16_le(&buffer[PACKET_LENGTH_OFFSET]);
    if (payload_length > PACKET_MAX_PAYLOAD) {
        return PKT_STATUS_INVALID_LENGTH;
    }

    expected_length = PACKET_HEADER_SIZE + payload_length + PACKET_CRC_SIZE;
    crc_length = PACKET_SEQUENCE_SIZE + PACKET_COMMAND_SIZE + PACKET_LENGTH_SIZE + payload_length;
    crc_offset = PACKET_HEADER_SIZE + payload_length;
    if (expected_length != buffer_length) {
        return PKT_STATUS_INVALID_LENGTH;
    }

    received_crc = packet_read_u32_le(&buffer[crc_offset]);
    calculated_crc = crc32_calculate(&buffer[PACKET_SOF_SIZE], crc_length);
    if (received_crc != calculated_crc) {
        return PKT_STATUS_INVALID_CRC;
    }

    // Packet is valid
    pkt->sequence_number = buffer[2];
    pkt->command = buffer[3];
    pkt->length = (uint16_t)payload_length;
    pkt->crc = received_crc;

    if (payload_length > 0U) {
        memcpy(pkt->data, &buffer[PACKET_HEADER_SIZE], payload_length);
    }

    return PKT_STATUS_OK;
}
