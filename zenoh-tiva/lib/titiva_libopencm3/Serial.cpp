#include "Serial.h"

class HardwareSerial Serial0(UART0);
class HardwareSerial Serial2(UART2); // UART1 connected to USB CDC port

// redirect printf to UART0 == USB CDC port
extern "C" int _write(int file, char *ptr, int len)
{
    (void)file;
    Serial0.write((uint8_t *)ptr, len);
    return len;
}

std::string bytes_to_hex(uint8_t *bytes, size_t length)
{
    std::string s;
    for (size_t i = 0; i < length; i++)
    {
        char buf[4];
        snprintf(buf, 4, "%02X ", bytes[i]);
        s += buf;
    }
    return s;
}
//=====================================================================================================
//=====================================================================================================

int HardwareSerial::begin(uint32_t baudrate)
{
    if (_usart == UART0)
    {
        // PA1 as TX and PA0 as RX
        periph_clock_enable(RCC_GPIOA);
        gpio_set_af(GPIOA, 1, GPIO0 | GPIO1); /* Mux PA0 and PA1 to UART0 (alternate function 1) */
        periph_clock_enable(RCC_UART0);
        /* We need a brief delay before we can access UART config registers */
        __asm__("nop");
    }
    else if (_usart == UART2)
    {
        // PD7 as TX and PD6 as RX
        periph_clock_enable(RCC_GPIOD);
        gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, GPIO7);
        gpio_mode_setup(GPIOD, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO6);
        gpio_set_af(GPIOD, 1, GPIO6 | GPIO7);
        periph_clock_enable(RCC_UART2);
        __asm__("nop");
    }
    else
    {
        return -1;
    }
    /* Disable the UART while we mess with its setings */
    uart_disable(_usart);
    /* Configure the UART clock source as precision internal oscillator */
    uart_clock_from_piosc(_usart);
    uart_set_baudrate(_usart, baudrate);
    uart_set_databits(_usart, 8);
    uart_set_parity(_usart, UART_PARITY_NONE);
    uart_set_stopbits(_usart, 1);
    uart_enable(_usart);

    /* Gimme and RX interrupt */
    uart_enable_rx_interrupt(_usart);
    uart_enable_tx_interrupt(_usart);
    switch (_usart)
    {
    case UART0:
    {
        nvic_enable_irq(NVIC_UART0_IRQ);
        break;
    }
    case UART2:
    {
        nvic_enable_irq(NVIC_UART2_IRQ);
        break;
    }
    }
    return 0;
}

int HardwareSerial::write(uint8_t *data, size_t length)
{
if ( _txBuffer.write(data, length) )
    {
        _txdOverflow++;
        return EOVERFLOW;
    }
    isr_txd_done();
    return 0;
}

void HardwareSerial::isr_txd_done()
{
    if (_txBuffer.hasData())
        uart_send(_usart, _txBuffer.read());
}

// split into PPP frames
void HardwareSerial::isr_rxd(uint8_t data) // ISR !
{
    _rxBuffer.write(data);
}

int HardwareSerial::available()
{
    return _rxBuffer.size();
}

uint8_t HardwareSerial::read()
{
    return _rxBuffer.read();
}

int HardwareSerial::end()
{

    return 0;
}

int HardwareSerial::flush()
{
    return 0;
}
//=====================================================================================================
//=====================================================================================================

/*
 * uart0_isr is declared as a weak function. When we override it here, the
 * libopencm3 build system takes care that it becomes our UART0 ISR.
 */
extern "C" void uart0_isr(void)
{
    uint8_t rx;
    uint32_t irq_clear = 0;

    if (uart_is_interrupt_source(UART0, UART_INT_RX))
    {
        rx = uart_recv(UART0);
        Serial0.isr_rxd(rx);
        irq_clear |= UART_INT_RX;
    }
    else if (uart_is_interrupt_source(UART0, UART_INT_TX))
    {
        Serial0.isr_txd_done();
        irq_clear |= UART_INT_TX;
    }

    uart_clear_interrupt_flag(UART0, (uart_interrupt_flag)irq_clear);
}

extern "C" void uart2_isr(void)
{
    uint8_t rx;
    uint32_t irq_clear = 0;

    if (uart_is_interrupt_source(UART2, UART_INT_RX))
    {
        rx = uart_recv(UART2);
        Serial2.isr_rxd(rx);
        irq_clear |= UART_INT_RX;
    }
    else if (uart_is_interrupt_source(UART2, UART_INT_TX))
    {
        Serial2.isr_txd_done();
        irq_clear |= UART_INT_TX;
    }

    uart_clear_interrupt_flag(UART2, (uart_interrupt_flag)irq_clear);
}