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

class Serial1 : public Serial
{

    size_t _rdPtr = 0;
    size_t _wrPtr = 0;
    uint8_t rx_buffer[2];
    std::vector<uint8_t> rx_data;
    uint32_t _rxdOverflow = 0;
    uint32_t _txdOverflow = 0;
    uint8_t tx_dma_buffer[128];
    bool _frame_complete = false;
    std::vector<uint8_t> _frameRxd;

public:
    UART_HandleTypeDef huart;
    DMA_HandleTypeDef hdma_usart_rx;
    DMA_HandleTypeDef hdma_usart_tx;
    uint8_t rx_dma_buffer[128];
    volatile bool crcDMAdone = true;


public:
    int begin(uint32_t baudrate);
    int end();
    int flush();
    int write(uint8_t *, size_t);
    int available();
    uint8_t read();

    void rx_isr();
    void rx_byte(uint8_t c);
};

class Serial2 : public Serial
{

    size_t _rdPtr = 0;
    size_t _wrPtr = 0;
    uint8_t rx_buffer[2];
    std::vector<uint8_t> rx_data;
    uint32_t _rxdOverflow = 0;
    uint32_t _txdOverflow = 0;
    uint8_t dma_buffer[128];
    uint8_t tx_dma_buffer[128];
    bool _frame_complete = false;
    std::vector<uint8_t> _frameRxd;

public:
    UART_HandleTypeDef huart;
    DMA_HandleTypeDef hdma_usart_rx;
    DMA_HandleTypeDef hdma_usart_tx;
    uint8_t rx_dma_buffer[128];
    volatile bool crcDMAdone = true;

public:
    int begin(uint32_t baudrate);
    int end();
    int flush();
    int write(uint8_t *, size_t);
    int available();
    uint8_t read();

    void rx_isr();
    void rx_byte(uint8_t c);
};

extern class Serial1 Serial1;
extern class Serial2 Serial2;

#endif /* _SERIAL_H_ */