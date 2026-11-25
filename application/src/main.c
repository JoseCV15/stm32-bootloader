#include "rcc.h"
#include "systick.h"
#include "uart.h"
#include "status.h"
#include "led.h"
#include "timer.h"

static led_t led_onboard = {
    .port = GPIO_PORT_A,
    .pin = GPIO_PIN_5,
    .polarity = true
};

int main()
{
    status_t res;
    uint8_t recv_byte;
    timer_t led_timer;

    systick_init(1000U);
    led_init(&led_onboard);
    led_off(&led_onboard);
    res = uart_init();
    if (res != STATUS_OK) {
        led_on(&led_onboard);
    }

    res = timer_init(&led_timer, 500U, TIMER_PERIODIC_MODE);
    if (res != STATUS_OK) {
        led_on(&led_onboard);
    }

    res = timer_start(&led_timer);
    if (res != STATUS_OK) {
        led_on(&led_onboard);
    }
    


    while (1) {

        if (timer_is_expired(&led_timer)) {
            res = uart_send_string(UART_ID_2, "Welcome to UART Echo example\r\n");
        }

    }

}