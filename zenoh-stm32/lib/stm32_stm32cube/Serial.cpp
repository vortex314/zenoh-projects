#include "Serial.h"

class Serial1 Serial1;
class Serial2 Serial2;


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

int Serial1::begin(uint32_t baudrate)
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

    // Configure DMA for USART1 TX
    __HAL_RCC_DMA2_CLK_ENABLE();
    __DMA2_CLK_ENABLE();
    hdma_usart_tx.Instance = DMA2_Stream7;
    hdma_usart_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart_tx.Init.Mode = DMA_NORMAL;
    hdma_usart_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    HAL_StatusTypeDef r = HAL_DMA_Init(&hdma_usart_tx);
    if (r != HAL_OK)
    {
        return r;
    }
    __HAL_LINKDMA(&huart, hdmatx, hdma_usart_tx);
    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    /*
        // Configure DMA for USART2 RX
        hdma_usart_rx.Instance = DMA2_Stream5;
        hdma_usart_rx.Init.Channel = DMA_CHANNEL_4;
        hdma_usart_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart_rx.Init.Mode = DMA_NORMAL;
        hdma_usart_rx.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_usart_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        HAL_DMA_Init(&hdma_usart_rx);
        if (r != HAL_OK)
        {
            return r;
        }
        __HAL_DMA_ENABLE_IT(&hdma_usart_rx, DMA_IT_RXNE);
        __HAL_LINKDMA(&huart, hdmarx, hdma_usart_rx);
        HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
    */
    // Enable USART1 clock
    __HAL_RCC_USART1_CLK_ENABLE();
    __USART1_CLK_ENABLE();
    // Configure USART1
    huart.Instance = USART1;
    huart.Init.BaudRate = baudrate;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    r = HAL_UART_Init(&huart);
    if (r != HAL_OK)
    {
        return r;
    };
    // interrupt for transmission complete or half complete
    __HAL_DMA_ENABLE_IT(&hdma_usart_tx, DMA_IT_TC | DMA_IT_HT);
    __HAL_UART_ENABLE_IT(&huart, USART_IT_RXNE);

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    HAL_UART_Receive_IT(&huart, rx_dma_buffer, 2);
    //    HAL_UART_Receive_DMA(&huart, rx_dma_buffer, sizeof(rx_dma_buffer));
    return 0;
}

int Serial1::write(uint8_t *data, size_t length)
{
    std::string s = bytes_to_hex(data, length);
    INFO("Serial1::write [%lu] [ %s]", length, s.c_str());

    if (!dma_done)
    {
        _txdOverflow++;
        return EOVERFLOW;
    }
    for (size_t i = 0; i < length; i++)
        tx_dma_buffer[i] = data[i];
    dma_done = false;
    if (HAL_UART_Transmit_DMA(&huart, tx_dma_buffer, length) != HAL_OK)
    {
        _txdOverflow++;
        return EOVERFLOW;
    }
    return 0;
}
// empty DMA buffer
void Serial1::rx_isr()
{
    _wrPtr = sizeof(rx_dma_buffer) - huart.hdmarx->Instance->NDTR;
    if (_wrPtr != _rdPtr)
    {
        if (_wrPtr > _rdPtr)
        {
            for (size_t i = _rdPtr; i < _wrPtr; i++)
                rx_byte(rx_dma_buffer[i]);
        }
        else
        {
            for (size_t i = _rdPtr; i < sizeof(rx_dma_buffer); i++)
                rx_byte(rx_dma_buffer[i]);
            for (size_t i = 0; i < _wrPtr; i++)
                rx_byte(rx_dma_buffer[i]);
        }
        _rdPtr = _wrPtr;
    }
}
// split into PPP frames
void Serial1::rx_byte(uint8_t c)
{
    if (c == COBS_SEPARATOR)
    {
        _frameRxd.push_back(c);
        _frame_complete = true;
    }
    else
    {
        if (_frameRxd.size() < FRAME_MAX)
        {
            _frameRxd.push_back(c);
        }
        else
        {
            _rxdOverflow++;
            _frameRxd.clear();
        }
    }
}

int Serial1::available()
{
    return _frame_complete;
}

uint8_t Serial1::read()
{
    uint8_t c = _frameRxd[0];
    _frameRxd.erase(_frameRxd.begin());
    if (_frameRxd.size() == 0)
        _frame_complete = false;
    return c;
}

int Serial1::end()
{
    HAL_UART_DeInit(&huart);
    HAL_DMA_DeInit(&hdma_usart_tx);
    HAL_NVIC_DisableIRQ(USART1_IRQn);
    return 0;
}

int Serial1::flush()
{
    return 0;
}
//=====================================================================================================
//=====================================================================================================

int Serial2::begin(uint32_t baudrate)
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

    // Configure DMA for USART2 TX
    __HAL_RCC_DMA1_CLK_ENABLE();
    __DMA1_CLK_ENABLE();
    hdma_usart_tx.Instance = DMA1_Stream6;
    hdma_usart_tx.Init.Channel = DMA_CHANNEL_4;
    hdma_usart_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart_tx.Init.Mode = DMA_NORMAL;
    hdma_usart_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    //   hdma_usart_tx.XferCpltCallback = HAL_UART_TxCpltCallback;
    HAL_StatusTypeDef r = HAL_DMA_Init(&hdma_usart_tx);
    if (r != HAL_OK)
    {
        return r;
    }
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    /*
        // Configure DMA for USART2 RX
        hdma_usart_rx.Instance = DMA1_Stream5;
        hdma_usart_rx.Init.Channel = DMA_CHANNEL_4;
        hdma_usart_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart_rx.Init.Mode = DMA_NORMAL;
        hdma_usart_rx.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_usart_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        r = HAL_DMA_Init(&hdma_usart_rx);
        if (r != HAL_OK)
        {
            return r;
        }
        HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);
        */
    // Enable USART2 clock
    __HAL_RCC_USART2_CLK_ENABLE();
    __USART2_CLK_ENABLE();
    // Configure USART2
    huart.Instance = USART2;
    huart.Init.BaudRate = baudrate;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    r = HAL_UART_Init(&huart);
    if (r != HAL_OK)
    {
        return r;
    };

    __HAL_LINKDMA(&huart, hdmatx, hdma_usart_tx);
    //   __HAL_LINKDMA(&huart, hdmarx, hdma_usart_rx);


    __HAL_DMA_ENABLE_IT(&hdma_usart_tx, DMA_IT_TC | DMA_IT_HT);
    __HAL_UART_ENABLE_IT(&huart, UART_IT_RXNE);

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    HAL_UART_Receive_IT(&huart, rx_dma_buffer, 1);

    //   HAL_UART_Receive_DMA(&huart, rx_dma_buffer, sizeof(rx_dma_buffer));
    return 0;
}

int Serial2::write(uint8_t *data, size_t length)
{

    //    return (int)HAL_UART_Transmit(&huart, data, length, HAL_MAX_DELAY);

    if (!dma_done)
    {
        _txdOverflow++;
        return EOVERFLOW;
    }
    for (size_t i = 0; i < length; i++)
        tx_dma_buffer[i] = data[i];
    dma_done = false;
    if (HAL_UART_Transmit_DMA(&huart, tx_dma_buffer, length) != HAL_OK)
    {
        _txdOverflow++;
        return EOVERFLOW;
    }
    return 0;
}
// empty DMA buffer
void Serial2::rx_isr()
{
    _wrPtr = sizeof(rx_dma_buffer) - huart.hdmarx->Instance->NDTR;
    if (_wrPtr != _rdPtr)
    {
        if (_wrPtr > _rdPtr)
        {
            for (size_t i = _rdPtr; i < _wrPtr; i++)
                rx_byte(rx_dma_buffer[i]);
        }
        else
        {
            for (size_t i = _rdPtr; i < sizeof(rx_dma_buffer); i++)
                rx_byte(rx_dma_buffer[i]);
            for (size_t i = 0; i < _wrPtr; i++)
                rx_byte(rx_dma_buffer[i]);
        }
        _rdPtr = _wrPtr;
    }
}
// split into PPP frames
void Serial2::rx_byte(uint8_t c)
{
    if (c == COBS_SEPARATOR)
    {
        _frame_complete = true;
    }
    else
    {
        if (_frameRxd.size() < FRAME_MAX)
        {
            _frameRxd.push_back(c);
        }
        else
        {
            _rxdOverflow++;
            _frameRxd.clear();
        }
    }
}

int Serial2::available()
{
    return _frame_complete;
}

uint8_t Serial2::read()
{
    uint8_t c = _frameRxd[0];
    _frameRxd.erase(_frameRxd.begin());
    return c;
}

int Serial2::end()
{
    HAL_UART_DeInit(&huart);
    HAL_DMA_DeInit(&hdma_usart_tx);
    HAL_NVIC_DisableIRQ(USART2_IRQn);
    return 0;
}

int Serial2::flush()
{
    return 0;
}

Serial *fromHandle(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
        return &Serial1;
    if (huart->Instance == USART2)
        return &Serial2;
    return nullptr;
}

extern "C" void UART_IDLECallback(UART_HandleTypeDef *huart)
{
    if (huart == &Serial1.huart)
        Serial1.rx_isr();
    if (huart == &Serial2.huart)
        Serial2.rx_isr();
}
// restart DMA as first before getting data when buffer overflows.
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Serial1.rx_byte(huart->Instance->DR);
        HAL_UART_Receive_IT(huart, Serial1.rx_dma_buffer, 1);
    }
    if (huart->Instance == USART2)
    {
        Serial1.rx_byte(huart->Instance->DR);
        HAL_UART_Receive_IT(huart, Serial2.rx_dma_buffer, 1);
    }
}

// get first half of buffer, to be ready before buffer full
extern "C" void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Serial1.rx_byte(huart->Instance->DR);
        HAL_UART_Receive_IT(huart, Serial1.rx_dma_buffer, 1);
    }
    if (huart->Instance == USART2)
    {
        Serial1.rx_byte(huart->Instance->DR);
        HAL_UART_Receive_IT(huart, Serial2.rx_dma_buffer, 1);
    }
}
/*extern "C" void HAL_UART_TxHalfCpltCallback(UART_HandleTypeDef *huart)
{
}*/
extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
        Serial1.dma_done = true;
    if (huart->Instance == USART2)
        Serial2.dma_done = true;
}

extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    volatile uint32_t rc = huart->ErrorCode;

    if (huart->Instance == USART1) // clear error flags
    {
        if (rc & HAL_UART_ERROR_ORE)
        {
            Serial1._rxdOverflow++;
            __HAL_UART_CLEAR_OREFLAG(huart);
            //            HAL_UART_Receive_DMA(huart, Serial1.rx_dma_buffer, sizeof(Serial1.rx_dma_buffer));
        }
    }
}

extern "C" void UART_DMAError(DMA_HandleTypeDef *hdma)
{
    PANIC("DMA error")
}

extern "C" void DMADoneCallback(DMA_HandleTypeDef *hdma)
{
    if (hdma->Instance == DMA2_Stream7)
        Serial1.dma_done = true;
    if (hdma->Instance == DMA1_Stream6)
        Serial2.dma_done = true;
}

extern "C" void DMA2_Stream7_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&Serial1.hdma_usart_tx);
}

extern "C" void DMA2_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&Serial1.hdma_usart_rx);
}

extern "C" void DMA1_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&Serial2.hdma_usart_tx);
}

extern "C" void DMA1_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&Serial2.hdma_usart_rx);
}

extern "C" void DMA_IRQHandler()
{
    HAL_DMA_IRQHandler(&Serial1.hdma_usart_tx);
    HAL_DMA_IRQHandler(&Serial2.hdma_usart_tx);
}

extern "C" void USART2_IRQHandler(void)
{
    HAL_UART_IRQHandler(&Serial2.huart); // calls HAL_UART_TxCpltCallback
}

extern "C" void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&Serial1.huart);
}