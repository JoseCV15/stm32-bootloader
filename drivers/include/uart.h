#ifndef UART_H
#define UART_H

#include <stdint.h>

#include "stm32l476xx.h"
#include "status.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UART_ID_1 = 0,
    UART_ID_2,
    UART_ID_3,

    UART_ID_MAX
} uart_id_t;

typedef enum {
    UART_WORD_LEN_8 = 0,
    UART_WORD_LEN_9,
    UART_WORD_LEN_7
} uart_word_len_t;

typedef enum {
    UART_PARITY_NONE = 0,
    UART_PARITY_EVEN,
    UART_PARITY_ODD
} uart_parity_t;

typedef enum {
    UART_STOP_BITS_0_5 = 0,
    UART_STOP_BITS_1,
    UART_STOP_BITS_1_5,
    UART_STOP_BITS_2
} uart_stop_bits_t;

typedef enum {
    UART_MODE_DISABLED,
    UART_MODE_TX,
    UART_MODE_RX,
    UART_MODE_TX_RX
} uart_mode_t;


typedef struct {
    uart_id_t uart_id;
    uint32_t baud_rate;
    uart_word_len_t data_size;
    uart_stop_bits_t stop_bits;
    uart_parity_t parity;
    uart_mode_t mode;
} uart_config_t;

status_t uart_init(void);
status_t uart_init_config(const uart_config_t *config);
status_t uart_deinit(uart_id_t uart_id);

status_t uart_send_byte(uart_id_t uart_id, uint8_t byte);
status_t uart_send_string(uart_id_t uart_id, const char *str);
status_t uart_send_buffer(uart_id_t uart_id, const uint8_t *buffer, uint32_t buffer_size);
status_t uart_receive_byte(uart_id_t uart_id, uint8_t *byte);
status_t uart_receive_string(uart_id_t uart_id, char *buffer_str, uint32_t buffer_size);

#ifdef __cplusplus
}
#endif



#endif // UART_H