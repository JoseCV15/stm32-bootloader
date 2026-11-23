#include "uart.h"
#include "gpio.h"
#include "rcc.h"
#include "ring_buffer.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Configuration */
#define UART_DEBUG_ID                   UART_ID_2
#define UART_DEBUG_BAUD_RATE            115200U
#define UART_RX_BUFFER_CAPACITY         512U
#define UART_DMA_RX_CAPACITY            128U

#define UART_DMA_RX_CHANNEL_NUMBER      6U
#define UART_DMA_RX_REQUEST             2U
#define UART_IRQ_PRIORITY               5U
#define UART_DMA_IRQ_PRIORITY           5U
#define UART_DMA_RX_CSELR_MASK          (0xFU << 20U)

typedef struct {
    gpio_port_t port;
    gpio_pin_mask_t pin;
    gpio_af_t af;
} uart_config_pin_t;

typedef enum {
    UART_CLK_PLCK1 = 0,
    UART_CLK_PLCK2
} uart_clk_src_t;

typedef struct {
    DMA_TypeDef *controller;
    DMA_Request_TypeDef *request;
    DMA_Channel_TypeDef *rx_channel;
    uint32_t rx_channel_number;
    uint32_t rx_request;
    IRQn_Type rx_irqn;
    uint32_t rx_buffer_size;
} uart_dma_config_t;

typedef struct {
    IRQn_Type uart_irqn;
    uint32_t uart_priority;
    IRQn_Type dma_rx_irqn;
    uint32_t dma_rx_priority;
}uart_nvic_config_t;

typedef struct {
    USART_TypeDef *instance;
    uart_clk_src_t clk_src;
    uart_config_pin_t tx;
    uart_config_pin_t rx;
    uart_dma_config_t dma;
    uart_nvic_config_t nvic;
} uart_hardware_t;

static const uart_hardware_t uart_hardware_table[UART_ID_MAX] = {
    [UART_ID_1] = {
        .instance = USART1,
        .clk_src = UART_CLK_PLCK2,
        .tx = {
            .port = GPIO_PORT_A,
            .pin = GPIO_PIN_9,
            .af = GPIO_AF_7
        },
        .rx = {
            .port = GPIO_PORT_A,
            .pin = GPIO_PIN_10,
            .af = GPIO_AF_7
        }
    },

    [UART_ID_2] = {
        .instance = USART2,
        .clk_src = UART_CLK_PLCK1,
        .tx = {
            .port = GPIO_PORT_A,
            .pin = GPIO_PIN_2,
            .af = GPIO_AF_7
        },
        .rx = {
            .port = GPIO_PORT_A,
            .pin = GPIO_PIN_3,
            .af = GPIO_AF_7
        },
        .dma  = {
            .controller = DMA1,
            .request = DMA1_CSELR,
            .rx_channel = DMA1_Channel6,
            .rx_channel_number = UART_DMA_RX_CHANNEL_NUMBER,
            .rx_request = UART_DMA_RX_REQUEST,
            .rx_irqn = DMA1_Channel6_IRQn,
            .rx_buffer_size = UART_DMA_RX_CAPACITY
        },
        .nvic = {
            .uart_irqn = USART2_IRQn,
            .uart_priority = UART_IRQ_PRIORITY,
            .dma_rx_irqn = DMA1_Channel6_IRQn,
            .dma_rx_priority = UART_DMA_IRQ_PRIORITY
        }
    },
    [UART_ID_3] = {
        .instance = USART3,
        .clk_src = UART_CLK_PLCK1,
        .tx = {
            .port = GPIO_PORT_B,
            .pin = GPIO_PIN_10,
            .af = GPIO_AF_7
        },
        .rx = {
            .port = GPIO_PORT_B,
            .pin = GPIO_PIN_11,
            .af = GPIO_AF_7
        }
    }
};

/* RX ring buffer: Debug UART (USART2) */
static ring_buffer_t uart_rx_rb = {0U};
static uint8_t uart_rx_buffer[UART_RX_BUFFER_CAPACITY] = {0U};

/* DMA circular buffer */
static uint8_t uart_dma_rx_buffer[UART_DMA_RX_CAPACITY] = {0U};

static volatile bool uart_rx_rb_initialized = false;
static uart_mode_t uart_mode[UART_ID_MAX];
static bool uart_initialized[UART_ID_MAX] = {false};

/* DMA buffer processing position */
static volatile uint32_t uart_dma_old_pos = 0U;

/* Diagnostics */
volatile uint32_t uart_dma_error_count = 0U;
volatile uint32_t uart_rx_dropped_count = 0U;
volatile uint32_t uart_rx_error_count = 0U;

/*------ Internal Functions ------*/
static bool uart_is_valid(uart_id_t uart_id) { return (uint32_t)uart_id < (uint32_t)UART_ID_MAX; }
static USART_TypeDef *uart_get_instance(uart_id_t uart_id) { return uart_hardware_table[uart_id].instance; }

static bool uart_has_dma(uart_id_t uart_id)
{
    const uart_hardware_t *hw;
    if (!uart_is_valid(uart_id)){
        return false;
    }
    hw = &uart_hardware_table[uart_id];

    return ((hw->dma.controller != NULL) && (hw->dma.request != NULL) &&
            (hw->dma.rx_channel != NULL));
}

static bool uart_mode_has_tx(uart_mode_t mode)
{
    return ((mode == UART_MODE_TX) || (mode == UART_MODE_TX_RX));
}

static bool uart_mode_has_rx(uart_mode_t mode)
{
    return ((mode == UART_MODE_RX) || (mode == UART_MODE_TX_RX));
}

static bool uart_config_is_valid(const uart_config_t *config)
{
    if (config == NULL) return false;
    if (!uart_is_valid(config->uart_id)) return false;
    if (config->baud_rate == 0U) return false;
    if (config->stop_bits > UART_STOP_BITS_2) return false;
    if (config->parity > UART_PARITY_ODD) return false;
    if (config->mode > UART_MODE_TX_RX) return false;
    return true;
}

/* Clock functions */
static uint32_t uart_get_clock(uart_id_t uart_id)
{
    if (!uart_is_valid(uart_id)) {
        return 0U;
    }
    uint32_t clk = 0U;
    switch (uart_hardware_table[uart_id].clk_src) {
    case UART_CLK_PLCK1: clk = rcc_get_apb1_clk(); break;
    case UART_CLK_PLCK2: clk = rcc_get_apb2_clk(); break;
    default: break;
    }
    return clk;
}

static status_t uart_enable_clock(uart_id_t uart_id)
{
    status_t status = STATUS_INVALID_ARGUMENT;
    switch (uart_id) {
    case UART_ID_1:
        rcc_enable_clk_peripheral(RCC_PERIPHERAL_USART1);
        status = STATUS_OK;
        break;
    case UART_ID_2:
        rcc_enable_clk_peripheral(RCC_PERIPHERAL_USART2);
        status = STATUS_OK;
        break;
    case UART_ID_3:
        rcc_enable_clk_peripheral(RCC_PERIPHERAL_USART3);
        status = STATUS_OK;
        break;
    default:
        break;
    }
    return status;
}

static status_t uart_disable_clock(uart_id_t uart_id)
{
    status_t status = STATUS_INVALID_ARGUMENT;
    switch (uart_id) {
    case UART_ID_1:
        rcc_disable_clk_peripheral(RCC_PERIPHERAL_USART1);
        status = STATUS_OK;
        break;
    case UART_ID_2:
        rcc_disable_clk_peripheral(RCC_PERIPHERAL_USART2);
        status = STATUS_OK;
        break;
    case UART_ID_3:
        rcc_disable_clk_peripheral(RCC_PERIPHERAL_USART3);
        status = STATUS_OK;
        break;
    default:
        break;
    }
    return status;
}

static status_t uart_dma_enable_clock(uart_id_t uart_id)
{
    const uart_hardware_t *hw;
    if (!uart_is_valid(uart_id)) {
        return STATUS_INVALID_ARGUMENT;
    }
    hw = &uart_hardware_table[uart_id];

    if (!uart_has_dma(uart_id)) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (hw->dma.controller == DMA1) {
        return rcc_enable_clk_peripheral(RCC_PERIPHERAL_DMA1);
    } else if (hw->dma.controller == DMA2) {
        return rcc_enable_clk_peripheral(RCC_PERIPHERAL_DMA2);
    } else {
        return STATUS_INVALID_ARGUMENT;
    }
}

/* GPIO configuration */

static status_t uart_configure_gpio(const uart_config_t *config_)
{
    uart_id_t uart_id = config_->uart_id;
    gpio_config_t config = {0U};
    status_t status;
    const uart_hardware_t *hw;

    if (!uart_is_valid(uart_id)) {
        return STATUS_INVALID_ARGUMENT;
    }

    hw = &uart_hardware_table[uart_id];

    // TX
    if (uart_mode_has_tx(config_->mode)) {
        config.port = hw->tx.port;
        config.pin_mask = hw->tx.pin;
        config.mode = GPIO_MODE_ALT_FUNC;
        config.otype = GPIO_OTYPE_PUSH_PULL;
        config.speed = GPIO_SPEED_HIGH;
        config.pull = GPIO_PULL_UP;
        config.af = hw->tx.af;

        status = gpio_init(&config);
        if (status != STATUS_OK) {
            return status;
        }
    }

    if (uart_mode_has_rx(config_->mode)) {
        config.port = hw->rx.port;
        config.pin_mask = hw->rx.pin;
        config.mode = GPIO_MODE_ALT_FUNC;
        config.otype = GPIO_OTYPE_PUSH_PULL;
        config.speed = GPIO_SPEED_HIGH;
        config.pull = GPIO_PULL_UP;
        config.af = hw->tx.af;

        status = gpio_init(&config);
        if (status != STATUS_OK) {
            return status;
        }
    }
    return STATUS_OK;
}

static uint32_t calc_baud_rate(uint32_t clock, uint32_t baudrate)
{
    if ((clock == 0U) || (baudrate == 0U)) {
        return 0U;
    }
    /* Oversampling by 16 */
    return ((clock + (baudrate /2U)) / baudrate);
}


static status_t uart_configure_peripheral(const uart_config_t *config)
{
    uint32_t clock, brr;
    USART_TypeDef *uart;
    uint32_t cr1 = 0U, cr2 = 0U;

    if (!uart_config_is_valid(config)) {
        return STATUS_INVALID_ARGUMENT;
    }

    uart = uart_get_instance(config->uart_id);
    clock = uart_get_clock(config->uart_id);

    if (!uart || (clock == 0U)) {
        return STATUS_ERROR;
    }

    brr = calc_baud_rate(clock, config->baud_rate);

    // For oversampling by 16, BRR must be within the valid usart divider range
    if ((brr < 0x10U) || (brr > 0xFFFFU)) {
        return STATUS_INVALID_ARGUMENT;
    }
    uart->CR1 &= ~USART_CR1_UE;
    uart->CR1 = 0U;
    uart->CR2 = 0U;
    uart->CR3 = 0U;

    uart->BRR = brr;

    switch(config->data_size) {
        case UART_WORD_LEN_7:
            cr1 |= USART_CR1_M1;
            break;
        case UART_WORD_LEN_8:
            break;
        case UART_WORD_LEN_9:
            cr1 |= USART_CR1_M0;
            break;
        default:
            return STATUS_INVALID_ARGUMENT;
    }

    switch(config->parity) {
        case UART_PARITY_NONE:
            break;
        case UART_PARITY_EVEN:
            cr1 |= USART_CR1_PCE;
            break;
        case UART_PARITY_ODD:
            cr1 |= USART_CR1_PCE | USART_CR1_PS;
            break;
        default:
            return STATUS_INVALID_ARGUMENT;
    }

    switch(config->stop_bits) {
        case UART_STOP_BITS_0_5:
            cr2 |= (1U << USART_CR2_STOP_Pos);
            break;
        case UART_STOP_BITS_1:
            break;
        case UART_STOP_BITS_1_5:
            cr2 |= (3U << USART_CR2_STOP_Pos);
            break;
        case UART_STOP_BITS_2:
            cr2 |= (2U << USART_CR2_STOP_Pos);
            break;
        default:
            return STATUS_INVALID_ARGUMENT;
    }

    if (uart_mode_has_tx(config->mode)) {
        cr1 |= USART_CR1_TE;
    }

    if (uart_mode_has_rx(config->mode)) {
        cr1 |= USART_CR1_RE;
    }

    uart->CR1 = cr1;
    uart->ICR = USART_ICR_IDLECF | USART_ICR_ORECF | USART_ICR_FECF |
                USART_ICR_NECF | USART_ICR_PECF;
    // Enable uart
    uart->CR1 |= USART_CR1_UE;

    return STATUS_OK;
}

/* DMA RX processing
* Copy new bytes from the DMA circular buffer to ring buffer.
* Should be called from usart2 and DMA1 channel 6
* Both DMA and UART interrupts should have the same premption priority to avoid race conditions
*/

static void uart_dma_rx_process(void)
{
    const uart_hardware_t *hw = &uart_hardware_table[UART_DEBUG_ID];
    uint32_t pos, old_pos, capacity, written;
    uint8_t byte;
    ring_buffer_status_t rb_status;

    if (!uart_rx_rb_initialized) {
        return;
    }

    capacity = hw->dma.rx_buffer_size;


    if ((capacity == 0U) || (capacity > UART_DMA_RX_CAPACITY)) {
        uart_dma_error_count++;
        return;
    }

    // In circular mode the CNDTR decreases as DMA writes bytes and wraps around 
    pos = capacity - hw->dma.rx_channel->CNDTR;

    if (pos >= capacity) {
        pos = 0U;
    }

    old_pos = uart_dma_old_pos;

    while (old_pos != pos) {
       // uint32_t written = 0U;
        byte = uart_dma_rx_buffer[old_pos];
        written = 0U;

        rb_status = ring_buffer_write(&uart_rx_rb, &byte, 1U, &written);

        if ((rb_status != RINGBUFFER_OK) || (written != 1U)) {
            uart_rx_dropped_count++;
        }
    
        old_pos++;

        if (old_pos >= capacity) {
            old_pos = 0U;
        }
    }
    uart_dma_old_pos = old_pos;
}

/* DMA RX initialization */
static status_t uart_dma_rx_init(uart_id_t uart_id)
{
    const uart_hardware_t *hw;
    DMA_Channel_TypeDef *dma_channel;
    uint32_t shift, cselr;
    status_t status;

    if (!uart_is_valid(uart_id)) {
        return STATUS_INVALID_ARGUMENT;
    }

    hw = &uart_hardware_table[uart_id];

    if (!uart_has_dma(uart_id)) {
        return STATUS_INVALID_ARGUMENT;
    }

    if ((hw->dma.rx_buffer_size == 0U) || (hw->dma.rx_buffer_size > UART_DMA_RX_CAPACITY) ||
       (hw->dma.rx_channel_number < 1U) || (hw->dma.rx_channel_number > 7U) || (hw->dma.rx_request > 0xFU)) {
        return STATUS_INVALID_ARGUMENT;
    }

    status = uart_dma_enable_clock(uart_id);

    if (status != STATUS_OK) {
        return status;
    }

    dma_channel = hw->dma.rx_channel;

    // Disable DMA channel before configuring
    dma_channel->CCR &= ~DMA_CCR_EN;

    while ((dma_channel->CCR & DMA_CCR_EN) != 0U) {
        // Wait for channel to be disabled
    }

    // Select dma request in CSELR
    shift = (hw->dma.rx_channel_number - 1U) * 4U;
    cselr = hw->dma.request->CSELR;
    cselr &= ~(0xFU << shift);
    cselr |= (hw->dma.rx_request << shift);

    hw->dma.request->CSELR = cselr;

    // Clear this channel flag
    hw->dma.controller->IFCR = (0xFU << ((hw->dma.rx_channel_number - 1U) * 4U));
    uart_dma_old_pos = 0U;
    
    // Peripheral address
    dma_channel->CPAR = (uint32_t)&(hw->instance->RDR);
    // Memory address: circular DMA buffer
    dma_channel->CMAR = (uint32_t)uart_dma_rx_buffer;
    // Number of bytes
    dma_channel->CNDTR = hw->dma.rx_buffer_size;
    /*
    * DIR = 00
    * CIRC = 1
    * MINC = 1
    * PSIZE = 00
    * MSIZE = 00
    * PL = 01
    */
    dma_channel->CCR = DMA_CCR_MINC | DMA_CCR_CIRC | DMA_CCR_PL_0 | 
                       DMA_CCR_HTIE | DMA_CCR_TCIE | DMA_CCR_TEIE ;
    
    // Configure DMA interrupt
    NVIC_SetPriority(hw->nvic.dma_rx_irqn, hw->nvic.dma_rx_priority);
    NVIC_ClearPendingIRQ(hw->nvic.dma_rx_irqn);
    NVIC_EnableIRQ(hw->nvic.dma_rx_irqn);

    // Configure usart interrupt for idle and error detection
    NVIC_SetPriority(hw->nvic.uart_irqn, hw->nvic.uart_priority);
    NVIC_ClearPendingIRQ(hw->nvic.uart_irqn);
    NVIC_EnableIRQ(hw->nvic.uart_irqn);
    
    // Enable DMA channel               
    dma_channel->CCR |= DMA_CCR_EN; // Enable channel
    hw->instance->CR3 |= USART_CR3_DMAR; // Enable UART DMA receiver

    hw->instance->ICR = USART_ICR_IDLECF;
    hw->instance->CR1 |= USART_CR1_IDLEIE; // Enable IDLE interrupt

    return STATUS_OK;
}

/* DMA RX deinitialization */
static void uart_dma_rx_deinit(uart_id_t uart_id)
{
    const uart_hardware_t *hw;

    if (!uart_is_valid(uart_id)) {
        return;
    }

    hw = &uart_hardware_table[uart_id];

    if (!uart_has_dma(uart_id)) {
        return;
    }

    // Stops usart dma requests and idle interrupt
    hw->instance->CR3 &= ~USART_CR3_DMAR;
    hw->instance->CR1 &= ~USART_CR1_IDLEIE;

    NVIC_DisableIRQ(hw->nvic.uart_irqn);
    NVIC_DisableIRQ(hw->nvic.dma_rx_irqn);
    // Disable DMA channel
    hw->dma.rx_channel->CCR &= ~(DMA_CCR_EN);
    while ((hw->dma.rx_channel->CCR & DMA_CCR_EN) != 0U) {
        // Wait for channel to be disabled
    }

    // Disable interrupts
    NVIC_ClearPendingIRQ(hw->nvic.uart_irqn);
    NVIC_ClearPendingIRQ(hw->nvic.dma_rx_irqn); 

    // Clear DMA interrupt flags
    hw->dma.controller->IFCR = (0xFU << ((hw->dma.rx_channel_number - 1U) * 4U));
    NVIC_ClearPendingIRQ(hw->nvic.uart_irqn);
    NVIC_ClearPendingIRQ(hw->nvic.dma_rx_irqn); 

    uart_dma_old_pos = 0U;
}

/* ------- Public Interface ------*/

status_t uart_init(void)
{
    static const uart_config_t default_config = {
        .uart_id = UART_DEBUG_ID,
        .baud_rate = UART_DEBUG_BAUD_RATE,
        .data_size = UART_WORD_LEN_8,
        .stop_bits = UART_STOP_BITS_1,
        .parity = UART_PARITY_NONE,
        .mode = UART_MODE_TX_RX
    };

    return uart_init_config(&default_config);
}

status_t uart_init_config(const uart_config_t *config)
{
    status_t status;

    if (!uart_config_is_valid(config)) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (uart_initialized[config->uart_id]) {
        return STATUS_ERROR;
    }

    // Prepare ring buffer for debug UART before enabling interrupt.
    if (config->uart_id == UART_DEBUG_ID) {
        uart_rx_rb_initialized = false;
        if (ring_buffer_init(&uart_rx_rb, uart_rx_buffer, UART_RX_BUFFER_CAPACITY) != RINGBUFFER_OK) {
            return STATUS_ERROR;
        }
        uart_dma_old_pos = 0U;
        uart_rx_dropped_count = 0U;
        uart_dma_error_count = 0U;
        uart_rx_error_count = 0U;
    }

    // Enable clock
    status = uart_enable_clock(config->uart_id);
    if (status != STATUS_OK) return status;

    // Configure GPIO
    status = uart_configure_gpio(config);
    if (status != STATUS_OK) {
        (void)uart_disable_clock(config->uart_id);
        return status;
    }

    // Configure USART
    status = uart_configure_peripheral(config);
    if (status != STATUS_OK) {
        (void)uart_disable_clock(config->uart_id);
        return status;
    } 

    if ((config->uart_id == UART_DEBUG_ID) && (uart_has_dma(config->uart_id) && (uart_mode_has_rx(config->mode)))) {
        //
        uart_rx_rb_initialized = true;
        status = uart_dma_rx_init(config->uart_id);

        if (status != STATUS_OK) {
            uart_rx_rb_initialized = false;
            USART2->CR1 &= ~USART_CR1_UE;
            (void)uart_disable_clock(config->uart_id);
            return status;
        }
    }

    uart_mode[config->uart_id] = config->mode;
    uart_initialized[config->uart_id] = true;

    return STATUS_OK;
}

status_t uart_deinit(uart_id_t uart_id)
{
    USART_TypeDef *uart;
    if (!uart_is_valid(uart_id)) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!uart_initialized[uart_id]) {
        return STATUS_INVALID_ARGUMENT;
    }

    uart = uart_get_instance(uart_id);

    if (!uart) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (uart_has_dma(uart_id)) {
        uart_rx_rb_initialized = false;
        uart_dma_rx_deinit(uart_id);
    }

    uart->CR1 &= ~USART_CR1_UE;

    uart->CR1 = 0U;
    uart->CR2 = 0U;
    uart->CR3 = 0U;

    uart_initialized[uart_id] = false;
    uart_mode[uart_id] = UART_MODE_DISABLED;
    return uart_disable_clock(uart_id);

}


status_t uart_send_byte(uart_id_t uart_id, uint8_t byte)
{
    USART_TypeDef *uart;

    if (!uart_is_valid(uart_id)) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!uart_initialized[uart_id]) {
        return STATUS_NOT_INITIALIZED;
    }

    if (!uart_mode_has_tx(uart_mode[uart_id])) {
        return STATUS_ERROR;
    }

    uart = uart_get_instance(uart_id);

    if (!uart) {
        return STATUS_INVALID_ARGUMENT;
    }

    while ((uart->ISR & USART_ISR_TXE) == 0U) {
    }

    uart->TDR = (uint32_t)byte;
    return STATUS_OK;
}

status_t uart_send_string(uart_id_t uart_id, const char *str)
{
    status_t status = STATUS_OK;
    if (!uart_is_valid(uart_id) || (!str)) {
        return STATUS_INVALID_ARGUMENT;
    }

    while (*str != '\0') {
        status = uart_send_byte(uart_id, (uint8_t)*str++);
        if (status != STATUS_OK) {
            return status;
        }
    }

    
    return STATUS_OK;
}


status_t uart_receive_byte(uart_id_t uart_id, uint8_t *byte)
{
    uint32_t received = 0U;
    ring_buffer_status_t rb_status;
    USART_TypeDef *uart;

    if (!uart_is_valid(uart_id) || !byte) {
        return STATUS_INVALID_ARGUMENT;
    }

    if (!uart_initialized[uart_id]) {
        return STATUS_NOT_INITIALIZED;
    }

    if (!uart_mode_has_rx(uart_mode[uart_id])) {
        return STATUS_ERROR;
    }

    if ((uart_id == UART_DEBUG_ID) && (uart_rx_rb_initialized)) {
         rb_status = ring_buffer_read(&uart_rx_rb, byte, 1U, &received);
        if (rb_status != RINGBUFFER_OK) {
            return STATUS_ERROR;
        }

        if (received != 1U) {
            return STATUS_ERROR;
        }
        return STATUS_OK;
    }

    /*
     * Other UART peripherals use polling or change it to interrupt only
     */
    uart = uart_get_instance(uart_id);

    if (uart == NULL) {
        return STATUS_ERROR;
    }

    if ((uart->ISR & USART_ISR_RXNE) == 0U) {
        return STATUS_ERROR;
    }

    *byte = (uint8_t)uart->RDR;

    return STATUS_OK;
}

status_t uart_receive_string(uart_id_t uart_id, char *buffer_str, uint32_t buffer_size)
{
    uint32_t index = 0U;
    uint8_t byte;
    status_t status = STATUS_OK;

    if (!uart_is_valid(uart_id) || !buffer_str || (buffer_size < 2U)) {
        return STATUS_INVALID_ARGUMENT;
    }
    
    while (index < (buffer_size -1U)) {
        status = uart_receive_byte(uart_id, &byte);
        if (status != STATUS_OK) {
            break;
        }
        if (byte == (uint8_t)'\n') {
            break;
        }
        
        buffer_str[index] = (char)byte;
        index++;
    }
    buffer_str[index] = '\0';

    if (index > 0U) {
        return STATUS_OK;
    }

    return status;

}


void USART2_IRQHandler(void)
{
    USART_TypeDef *uart = uart_get_instance(UART_DEBUG_ID);
    uint32_t isr;

    if (!uart) {
        return;
    }

    isr = uart->ISR;

    if ((isr & USART_ISR_IDLE) != 0U) {
        uart->ICR = USART_ICR_IDLECF;
        uart_dma_rx_process();
    }

    // Overrun error 
    if ((isr & USART_ISR_ORE) != 0U) {
        uart->ICR = USART_ICR_ORECF;
        uart_rx_error_count++;
    }

    // Framing error 
    if ((isr & USART_ISR_FE) != 0U) {
        uart->ICR = USART_ICR_FECF;
        uart_rx_error_count++;
    }

    // Noise error 
    if ((isr & USART_ISR_NE) != 0U) {
        uart->ICR = USART_ICR_NECF;
        uart_rx_error_count++;
    }

    // Parity error 
    if ((isr & USART_ISR_PE) != 0U) {
        uart->ICR = USART_ICR_PECF;
        uart_rx_error_count++;
    }
}


void DMA1_Channel6_IRQHandler(void)
{
    const uart_hardware_t *hw = &uart_hardware_table[UART_DEBUG_ID];
    DMA_TypeDef *dma = hw->dma.controller;
    uint32_t flags;
    uint32_t clear_flags = 0U;

    if (!dma) {
        return;
    }

    flags = dma->ISR;

    /* Half-transfer */
    if ((flags & DMA_ISR_HTIF6) != 0U) {
        clear_flags |= DMA_IFCR_CHTIF6;
    }

    /* Transfer complete */
    if ((flags & DMA_ISR_TCIF6) != 0U) {
        clear_flags |= DMA_IFCR_CTCIF6;
    }

    /* Transfer error */
    if ((flags & DMA_ISR_TEIF6) != 0U) {
        clear_flags |= DMA_IFCR_CTEIF6;
        uart_dma_error_count++;
    }

    /* Clear handled interrupt flags. */
    if (clear_flags != 0U) {
        dma->IFCR = clear_flags;
    }

    /* Stop DMA on transfer error. */
    if ((flags & DMA_ISR_TEIF6) != 0U) {

        hw->instance->CR3 &= ~USART_CR3_DMAR;

        hw->dma.rx_channel->CCR &= ~DMA_CCR_EN;

        return;
    }

    /* Copy newly received bytes from DMA to the ring buffer. */
    if ((flags & (DMA_ISR_HTIF6 | DMA_ISR_TCIF6)) != 0U) {
        uart_dma_rx_process();
    }
}