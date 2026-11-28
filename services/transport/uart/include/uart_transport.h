#ifndef UART_TRANSPORT_H
#define UART_TRANSPORT_H

#include <stddef.h>
#include <stdint.h>
#include "status.h"


/**
* @brief Receives all available bytes
* @param buffer       In-memory buffer
* @param buffer_size  Maximum buffer capacity
* @param received     Number of bytes 
*
* @return STATUS_OK  on success.
*/
status_t uart_transport_receive(uint8_t *buffer, size_t buffer_size, size_t *received);

/**
 * @brief Transmit block of bytes
 * @param buffer Data to transmit
 * @param length Number of bytes to transmit
 * 
 * @return STATUS_OK on success
 */
status_t uart_transport_send(const uint8_t *buffer, size_t length);

#endif // UART_TRANSPORT_H