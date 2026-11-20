#include <stdint.h>
#include <stddef.h>

#include "stm32l476xx.h"

#define GPIO_HIGH        1U
#define GPIO_LOW         0U
#define LED_PIN          5U
#define LED_GPIO         GPIOA

#define CLEAR_BIT(reg, bit)     ((reg) &= (1U << (bit)))
#define SET_BIT(reg, bit)       ((reg) |= (1U << (bit)))

static void blocking_delay(volatile uint32_t count)
{
    while (count -- > 0U)
    {
        __NOP();
    }
}

static void gpio_configure_clock(void)
{
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
    (void)RCC->AHB2ENR;
}

static void gpio_init(GPIO_TypeDef *port, uint8_t pin)
{
    // Moder as output
    port->MODER &= ~(3UL << (pin * 2U));
    port->MODER |= (1UL << (pin * 2U));

    // Push-pull output
    port->OTYPER &= ~(1UL << pin);

    // Low speed output
    port->OSPEEDR &= ~(3UL << (pin * 2U));

    // No pull-up/pull-down
    port->PUPDR &= ~(3UL << (pin * 2U));
}

/** @brief Set gpio level
 *  @param level if 1 set gpio to high other if 0 set it to low
 */
static void gpio_set_level(GPIO_TypeDef *port, uint8_t pin, uint8_t level)
{
    if ((port == NULL) || (pin > 15U)) {
        return;
    }

    if (level == 1U) {
        port->BSRR = (1UL << pin);
    }
    else if (level == 0U) {
        port->BSRR = (1UL << (pin + 16U));
    }
    else {
        return;
    }
}


int main()
{

    gpio_configure_clock();
    gpio_init(LED_GPIO, LED_PIN);


    while (1) {
        gpio_set_level(LED_GPIO, LED_PIN, GPIO_HIGH);
        blocking_delay(800000U);
        gpio_set_level(LED_GPIO, LED_PIN, GPIO_LOW);
        blocking_delay(800000U);
    }

    return 0;
}