#include "rcc.h"
#include "systick.h"
#include "uart.h"
#include "status.h"
#include "led.h"

static led_t led_onboard = {
    .port = GPIO_PORT_A,
    .pin = GPIO_PIN_5,
    .polarity = true
};

int main()
{
    status_t res;
    uint8_t recv_byte;

    systick_init(1000U);
    led_init(&led_onboard);
    led_off(&led_onboard);
    res = uart_init();
    if (res != STATUS_OK) {
        led_on(&led_onboard);
    }

    res = uart_send_string(UART_ID_2, "Welcome to UART Echo example\r\n");
    if (res != STATUS_OK) {
        led_on(&led_onboard);
    }

    while (1) {
        
        res = uart_receive_byte(UART_ID_2, &recv_byte);
        if (res == STATUS_OK) {
            // echoed + 1
            uart_send_byte(UART_ID_2, recv_byte +1);
        }
    }

}