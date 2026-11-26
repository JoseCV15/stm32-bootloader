#include "packet.h"
#include "crc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_crc32(void)
{
    static const uint8_t test_data[] = "123456789";
    uint32_t crc = crc32_calculate(test_data, sizeof(test_data) - 1);
    printf("Calculated CRC: 0x%08lX\n", (unsigned long)crc);
    assert(crc == 0xCBF43926UL);
    printf("CRC32 test: OK\n");
}

static void print_buffer(const uint8_t *data)
{
    for (size_t i = 0; i < sizeof(data); i++) {
        printf(" %x - ", data[i]);
    }
}

static void test_packet_encode_decode(void)
{
    packet_t tx_pkt = {0};
    packet_t rx_pkt = {0};
    uint8_t buffer[PACKET_MAX_SIZE] = {0};
    size_t encoded_length = 0U;

    tx_pkt.sequence_number = 5U;
    tx_pkt.command = PACKET_CMD_WRITE;
    tx_pkt.length = 4U;

    tx_pkt.data[0] = 0x11U;
    tx_pkt.data[1] = 0x22U;
    tx_pkt.data[2] = 0x33U;
    tx_pkt.data[3] = 0x44U;

    packet_status_t status = packet_encode(&tx_pkt, buffer, sizeof((size_t)buffer), &encoded_length);
    assert(status == PKT_STATUS_OK);
    assert(encoded_length == 14U); 
    print_buffer(buffer);
    printf("Packet encode test OK\n");

    status = packet_decode(buffer, encoded_length, &rx_pkt);
    assert(status == PKT_STATUS_OK);

    assert(rx_pkt.sequence_number == tx_pkt.sequence_number);
    assert(rx_pkt.command == tx_pkt.command);
    assert(rx_pkt.length == tx_pkt.length);

    assert(memcmp(rx_pkt.data, tx_pkt.data, tx_pkt.length) == 0);

    printf("Packet encode/decode test: OK\n");  
}

static void test_packet_invalid_crc(void)
{
    packet_t tx_pkt = {0};
    packet_t rx_pkt = {0};
    uint8_t buffer[PACKET_MAX_SIZE] = {0};
    size_t encoded_length = 0U;

    tx_pkt.sequence_number = 1U;
    tx_pkt.command = PACKET_CMD_GET_INFO;
    tx_pkt.length = 1U;

    tx_pkt.data[0] = 0x55U;

    packet_status_t status = packet_encode(&tx_pkt, buffer, sizeof((size_t)buffer), &encoded_length);
    assert(status == PKT_STATUS_OK);
    
    buffer[PACKET_HEADER_SIZE] ^= 0x01U;

    status = packet_decode(buffer, encoded_length, &rx_pkt);
    assert(status == PKT_STATUS_INVALID_CRC);

    printf("Packet invalid crc test: OK\n");  
}


int main(void)
{
    test_crc32();
    test_packet_encode_decode();
    test_packet_invalid_crc();

    printf("\nAll packet tests passed!\n");

    return 0;
}