#include <common.h>

uint64_t pico_ms_tick = 0; // Global variable to keep track of milliseconds

extern "C" void SysTick_Handler(void)
{
  pico_ms_tick++; // Increment the millisecond tick counter
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

extern "C" void DMA1_Channel2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
}

extern "C" void DMA1_Channel3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
}



extern "C" void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
  if (RESET != __HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_IDLE))
  {                                     // Check for IDLE line interrupt
    __HAL_UART_CLEAR_IDLEFLAG(&huart3); // Clear IDLE line flag (otherwise it will continue to enter interrupt)
    usart3_rx_check();                                        // Check for data to process
  }
}

extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {
    // Process the received data in rx_buffer_R
    // For example, you can print it or handle it as needed
    // Here we just reset the DMA to receive more data
    HAL_UART_Receive_DMA(&huart3, (uint8_t *)rx_buffer_R, sizeof(rx_buffer_R));
  }
}

extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {
    // Transmission complete callback
    // You can add code here to handle post-transmission actions if needed
  }
}

extern "C" void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART3)
  {
    // Handle UART error
    // You can add code here to handle errors such as parity error, framing error, etc.
    // For example, you can reset the UART or log the error
  }
}
