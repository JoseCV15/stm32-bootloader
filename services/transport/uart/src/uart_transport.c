#include "uart_transport.h"
#include "uart.h"

#define UART_TRANSPORT_ID           UART_ID_2

status_t uart_transport_receive(uint8_t *buffer, size_t buffer_size, size_t *received)
{
    status_t status;
    size_t count = 0U;
    uint8_t byte;

    if(!buffer || (buffer_size == 0U) || !received) {
        return STATUS_INVALID_ARGUMENT;
    }

    *received = 0U;

    while (count < buffer_size) {
        status = uart_receive_byte(UART_TRANSPORT_ID, &byte);
        if (status != STATUS_OK) {
            break;
        }
        buffer[count] = byte;
        count++;
    }

    *received = count;
    return STATUS_OK;
}

status_t uart_transport_send(const uint8_t *buffer, size_t length)
{
    if(!buffer || (length == 0U)) {
        return STATUS_INVALID_ARGUMENT;
    }

    return uart_send_buffer(UART_TRANSPORT_ID, buffer, (uint32_t)length);

}