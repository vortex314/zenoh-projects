#include "Serial.h"

class HardwareSerial Serial0(0);
class HardwareSerial Serial1(1); // UART1 connected to USB CDC port 

// redirect printf to UART2
extern "C" int _write(int file, char *ptr, int len)
{
    (void)file;
    Serial1.write((uint8_t *)ptr, len);
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
    	/* Enable GPIOA in run mode. */
	periph_clock_enable(RCC_GPIOA);
	/* Mux PA0 and PA1 to UART0 (alternate function 1) */
	gpio_set_af(GPIOA, 1, GPIO0 | GPIO1);

	/* Enable the UART clock */
	periph_clock_enable(RCC_UART0);
	/* We need a brief delay before we can access UART config registers */
	__asm__("nop");
	/* Disable the UART while we mess with its setings */
	uart_disable(UART0);
	/* Configure the UART clock source as precision internal oscillator */
	uart_clock_from_piosc(UART0);
	/* Set communication parameters */
	uart_set_baudrate(UART0, baudrate);
	uart_set_databits(UART0, 8);
	uart_set_parity(UART0, UART_PARITY_NONE);
	uart_set_stopbits(UART0, 1);
	/* Now that we're done messing with the settings, enable the UART */
	uart_enable(UART0);

    	/* Gimme and RX interrupt */
	uart_enable_rx_interrupt(UART0);
	/* Make sure the interrupt is routed through the NVIC */
	nvic_enable_irq(NVIC_UART0_IRQ);
    return 0;
}

int HardwareSerial::write(uint8_t *data, size_t length)
{
    if (_usart == 1)
    {
        INFO("Txd [%d]", length);
    }

    bool was_empty = _txBuffer.size() == 0;
    for (size_t i = 0; i < length ; i++)
    {
        if (_txBuffer.write(data[i]) != 0)
        {
            _txdOverflow++;
            return EOVERFLOW;
        }
    }
    if (was_empty)
    {
        isr_txd_done();
    }
    return 0;
}

void HardwareSerial::isr_txd_done()
{
    int sbuf_cnt = 0;
    for (size_t i = 0; i < sizeof(_sbuf) && _txBuffer.hasData(); i++)
    {
        _sbuf[i] = _txBuffer.read();
        sbuf_cnt++;
    }
 //   if (sbuf_cnt)
   //     HAL_UART_Transmit_IT(&huart, _sbuf, sbuf_cnt);
}

// split into PPP frames
void HardwareSerial::isr_rxd(uint16_t size) // ISR !
{
    for (size_t i = 0; i < size; i++)
    {
        _rxBuffer.write(_rbuf[i]);
    }
  //  HAL_UARTEx_ReceiveToIdle_IT(&huart, _rbuf, sizeof(_rbuf));
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
void uart0_isr(void)
{
	uint8_t rx;
	uint32_t irq_clear = 0;

	if (uart_is_interrupt_source(UART0, UART_INT_RX)) {
		rx = uart_recv(UART0);
		uart_send(UART0, rx);
		irq_clear |= UART_INT_RX;
	}

	uart_clear_interrupt_flag(UART0, (uart_interrupt_flag)irq_clear);
}