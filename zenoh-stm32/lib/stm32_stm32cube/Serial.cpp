#include "Serial.h"

class HardwareSerial Serial1(USART1);
class HardwareSerial Serial2(USART2);

// redirect printf to UART2
extern "C" int _write(int file, char *ptr, int len)
{
    Serial2.write((uint8_t *)ptr, len);
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
    if (_usart == USART1)
    {
        // USART1 GPIO Configuration
        // PA9     ------> USART1_TX
        // PA10     ------> USART1_RX
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        __HAL_RCC_USART1_CLK_ENABLE();
        __USART1_CLK_ENABLE();
    }
    else if (_usart == USART2)
    {
        // USART2 GPIO Configuration
        // PA2     ------> USART2_TX
        // PA3     ------> USART2_RX
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        __HAL_RCC_USART2_CLK_ENABLE();
        __USART2_CLK_ENABLE();
    }
    else
    {
        return -1;
    }

    // Configure USART1
    huart.Instance = _usart;
    huart.Init.BaudRate = baudrate;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_StatusTypeDef r = HAL_UART_Init(&huart);
    if (r != HAL_OK)
    {
        return r;
    };
    __HAL_UART_ENABLE_IT(&huart, USART_IT_TC | USART_IT_IDLE | USART_IT_RXNE);
    HAL_UARTEx_ReceiveToIdle_IT(&huart, _rbuf, sizeof(_rbuf) / 2);

    if (_usart == USART1)
    {
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
    if (_usart == USART2)
    {
        HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART2_IRQn);
    }
    return 0;
}

int HardwareSerial::write(uint8_t *data, size_t length)
{
    if (_usart == USART1)
    {
        INFO("Txd [%d] [%s]", length,bytes_to_hex(data, length).c_str());
    }

    bool was_empty = _txBuffer.size() == 0;
    for (size_t i = 0; i < length; i++)
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
    if (sbuf_cnt)
        HAL_UART_Transmit_IT(&huart, _sbuf, sbuf_cnt);
}

// split into PPP frames
void HardwareSerial::isr_rxd(uint16_t size) // ISR !
{
    HAL_UARTEx_ReceiveToIdle_IT(&huart, _rbuf, sizeof(_rbuf));
    for (size_t i = 0; i < size; i++)
    {
        _rxBuffer.write(_rbuf[i]);
    }
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
    if (_usart == USART1)
    {
        __HAL_RCC_USART1_CLK_DISABLE();
        __USART1_CLK_DISABLE();
        HAL_UART_DeInit(&huart);
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
    else if (_usart == USART2)
    {
        __HAL_RCC_USART2_CLK_DISABLE();
        __USART2_CLK_DISABLE();
        HAL_UART_DeInit(&huart);
        HAL_NVIC_DisableIRQ(USART2_IRQn);
    }
    return 0;
}

int HardwareSerial::flush()
{
    return 0;
}
//=====================================================================================================
//=====================================================================================================

extern "C" void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
        Serial1.isr_rxd(Size);
    if (huart->Instance == USART2)
        Serial2.isr_rxd(Size);
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
        Serial1.isr_txd_done();
    else if (huart->Instance == USART2)
        Serial2.isr_txd_done();
}
// not sure the below is of any help
extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    volatile uint32_t rc = huart->ErrorCode;

    if (rc & HAL_UART_ERROR_ORE)
    {
        if (huart->Instance == USART1)
            Serial1._rxdOverflow++;
        if (huart->Instance == USART2)
            Serial2._rxdOverflow++;
        __HAL_UART_CLEAR_OREFLAG(huart);
    }
    if (rc & HAL_UART_ERROR_NE)
    {
        __HAL_UART_CLEAR_NEFLAG(huart);
    }
    if (rc & HAL_UART_ERROR_FE)
    {
        __HAL_UART_CLEAR_FEFLAG(huart);
    }
    if (rc & HAL_UART_ERROR_PE)
    {
        __HAL_UART_CLEAR_PEFLAG(huart);
    }
    HAL_UART_Receive_IT(huart, (uint8_t *)&rc, 1);
}

extern "C" void USART2_IRQHandler(void)
{
    // stm32cube doesn't handle this condition correctly
    if (READ_REG(Serial2.huart.Instance->SR) & USART_SR_ORE)
    {
        __HAL_UART_CLEAR_OREFLAG(&Serial2.huart);
    }
    HAL_UART_IRQHandler(&Serial2.huart);
}

extern "C" void USART1_IRQHandler(void)
{
    // stm32cube doesn't handle this condition correctly
    if (READ_REG(Serial1.huart.Instance->SR) & USART_SR_ORE)
    {
        __HAL_UART_CLEAR_OREFLAG(&Serial1.huart);
    }
    HAL_UART_IRQHandler(&Serial1.huart);
}