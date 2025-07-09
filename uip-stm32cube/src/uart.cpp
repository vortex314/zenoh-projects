#include <common.h>

void UART3_Init(void)
{
    /* DMA controller clock enable */
    __HAL_RCC_DMA1_CLK_ENABLE();

    /* DMA interrupt init */
    /* DMA1_Channel2_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    /* DMA1_Channel3_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);

    huart3.Instance = USART3;
    huart3.Init.BaudRate = USART3_BAUD;
    huart3.Init.WordLength = USART3_WORDLENGTH;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);
}

void UART_DisableRxErrors(UART_HandleTypeDef *huart)
{
    CLEAR_BIT(huart->Instance->CR1, USART_CR1_PEIE); /* Disable PE (Parity Error) interrupts */
    CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);  /* Disable EIE (Frame error, noise error, overrun error) interrupts */
}

extern "C" void HAL_UART_MspInit(UART_HandleTypeDef *uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == USART3)
    {
        /* USER CODE BEGIN USART3_MspInit 0 */

        /* USER CODE END USART3_MspInit 0 */
        /* USART3 clock enable */
        __HAL_RCC_USART3_CLK_ENABLE();

        __HAL_RCC_GPIOB_CLK_ENABLE();
        /**USART3 GPIO Configuration
        PB10     ------> USART3_TX
        PB11     ------> USART3_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_11;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* USART3 DMA Init */
        /* USART3_RX Init */
        hdma_usart3_rx.Instance = DMA1_Channel3;
        hdma_usart3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_usart3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart3_rx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart3_rx.Init.Mode = DMA_CIRCULAR;
        hdma_usart3_rx.Init.Priority = DMA_PRIORITY_LOW;
        HAL_DMA_Init(&hdma_usart3_rx);
        __HAL_LINKDMA(uartHandle, hdmarx, hdma_usart3_rx);

        /* USART3_TX Init */
        hdma_usart3_tx.Instance = DMA1_Channel2;
        hdma_usart3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hdma_usart3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_usart3_tx.Init.MemInc = DMA_MINC_ENABLE;
        hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
        hdma_usart3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
        hdma_usart3_tx.Init.Mode = DMA_NORMAL;
        hdma_usart3_tx.Init.Priority = DMA_PRIORITY_LOW;
        HAL_DMA_Init(&hdma_usart3_tx);
        __HAL_LINKDMA(uartHandle, hdmatx, hdma_usart3_tx);

        /* USART3 interrupt Init */
        HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
        /* USER CODE BEGIN USART3_MspInit 1 */
        __HAL_UART_ENABLE_IT(uartHandle, UART_IT_IDLE); // Enable the USART IDLE line detection interrupt
                                                        /* USER CODE END USART3_MspInit 1 */
    }
}

extern "C" void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART3)
  {

    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
  
    /**USART3 GPIO Configuration    
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX 
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 DMA DeInit */
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);

  }
} 

void usart3_rx_check()
{
    static uint32_t old_pos = 0;
    uint32_t pos;
    pos = rx_buffer_R_len - __HAL_DMA_GET_COUNTER(huart3.hdmarx); // Calculate current position in buffer

    uint8_t ptr_debug[RX_BUFFER_SIZE];

    if (pos != old_pos)
    { // Check change in received data
        if (pos > old_pos)
        {                                                           // "Linear" buffer mode: check if current position is over previous one
            process_rx_bytes(&rx_buffer_R[old_pos], pos - old_pos); // Process data
        }
        else
        {                                                                            // "Overflow" buffer mode
            memcpy(&ptr_debug[0], &rx_buffer_R[old_pos], rx_buffer_R_len - old_pos); // First copy data from the end of buffer
            if (pos > 0)
            {                                                                        // Check and continue with beginning of buffer
                memcpy(&ptr_debug[rx_buffer_R_len - old_pos], &rx_buffer_R[0], pos); // Copy remaining data
            }
            process_rx_bytes(ptr_debug, rx_buffer_R_len - old_pos + pos); // Process data
        }
    }
    old_pos = pos; // Update old position
    if (old_pos == rx_buffer_R_len)
    { // Check and manually update if we reached end of buffer
        old_pos = 0;
    }
}
