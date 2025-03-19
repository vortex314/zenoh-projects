#ifndef _SERIAL_H_
#define _SERIAL_H_

#include <errno.h>
#include <stdint.h>
#include <stm32f401xe.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_adc.h>
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_usart.h"
#include <util.h>
#include <sys.h>
#include <CircBuf.h>

#define COBS_SEPARATOR 0x00
#define FRAME_MAX 256

class Serial
{
public:
    virtual int begin(uint32_t baudrate) = 0;
    virtual int end() = 0;
    virtual int flush() = 0;
    virtual int write(uint8_t *, size_t) = 0;
    virtual int available() = 0;
    virtual uint8_t read() = 0;
};

class HardwareSerial : public Serial
{

public:
    UART_HandleTypeDef huart;
    uint32_t _txdOverflow = 0;
    uint32_t _rxdOverflow = 0;
    CircBuf _rxBuffer;
    CircBuf _txBuffer;
    USART_TypeDef* _usart;
    uint8_t sbuf[4];
    size_t sbuf_cnt=0;
    uint8_t rbuf[4];
    size_t rbuf_cnt=0;

public:
    HardwareSerial(USART_TypeDef* usart) : _rxBuffer(512), _txBuffer(512), _usart(usart) {}
    int begin(uint32_t baudrate);
    int end();
    int flush();
    int write(uint8_t *, size_t);
    int available();
    uint8_t read();

    void isr_txd_done();
    void isr_rxd(uint16_t cnt);
};

extern class HardwareSerial Serial1;
extern class HardwareSerial Serial2;

#endif /* _SERIAL_H_ */