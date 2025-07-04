#include <stdint.h>
#include <stm32f1xx_hal.h>        // Include the HAL library for STM32F1 series
#include <stm32f1xx_hal_def.h>    // Include the HAL definitions for STM32F1 series
#include <stm32f1xx_hal_cortex.h> // Include the HAL Cortex functions for STM32F1 series
#include <stm32f1xx_hal_rcc.h>    // Include the HAL RCC functions for STM32F1 series
#include <stm32f1xx_hal_uart.h>
#include <cstring>
#include <cstdio>
extern "C"
{
#include <uip.h>

#include <timer.h>
};

void UART3_Init(void); // Function prototype for UART2 initialization
void SystemClock_Config(void);
void UART_DisableRxErrors(UART_HandleTypeDef *huart);

#define USART3_BAUD 115200                   // UART2 baud rate (long wired cable)
#define USART3_WORDLENGTH UART_WORDLENGTH_8B // UART_WORDLENGTH_8B or UART_WORDLENGTH_9B
UART_HandleTypeDef huart3;                   // UART handle for UART3
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
#define RX_BUFFER_SIZE 64       // Define the size of the receive buffer
u8 rx_buffer_R[RX_BUFFER_SIZE]; // Receive buffer for UART2

const char *line = "The quick brown fox jumps over the lazy dog. 1234567890\n\r"; // Define a line ending for UART communication

extern "C" void uip_callback() {}

extern "C" void uip_log(char *s) {}




#include <ppp.cpp>
// Example platform-specific implementation
class PPP_Serial : public PPP {
public:
    void serial_send(const uint8_t* data, uint16_t len) override {
         HAL_UART_Transmit_DMA(&huart3, data, len);

    }
    
    void uip_input_packet(const uint8_t* packet, uint16_t len) override {
        // Forward packet to uIP stack
        // memcpy(uip_buf, packet, len);
        // uip_len = len;
        uip_input();
    }
};
PPP_Serial ppp_serial; // Create an instance of the PPP_Serial class

void network_device_init()
{
  HAL_Init();
  __HAL_RCC_AFIO_CLK_ENABLE();
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  /* System interrupt init*/
  /* MemoryManagement_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(MemoryManagement_IRQn, 0, 0);
  /* BusFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(BusFault_IRQn, 0, 0);
  /* UsageFault_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(UsageFault_IRQn, 0, 0);
  /* SVCall_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SVCall_IRQn, 0, 0);
  /* DebugMonitor_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DebugMonitor_IRQn, 0, 0);
  /* PendSV_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(PendSV_IRQn, 0, 0);
  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  SystemClock_Config();

  __HAL_RCC_DMA1_CLK_DISABLE();
  UART3_Init(); // Initialize UART2 with the defined settings
  HAL_UART_Receive_DMA(&huart3, (uint8_t *)rx_buffer_R, sizeof(rx_buffer_R));
  UART_DisableRxErrors(&huart3);
  // This is a simple C++ program that does nothing.
  // It serves as a placeholder for future development.
  ppp_serial.init(); // Initialize the PPP instance
  // Set the PPP state to establish connection
  
}

void network_device_send()
{
  ppp_serial.send_frame(PPP_IP, uip_buf, uip_len);
}

int network_device_read() { return 0; }


extern "C" int main()
{
  uip_ipaddr_t ipaddr;
  struct timer periodic_timer;

  timer_set(&periodic_timer, CLOCK_SECOND / 2);

  network_device_init();
  uip_init();

  uip_ipaddr(ipaddr, 192, 168, 0, 2);
  uip_sethostaddr(ipaddr);

  while (1)
  {
    uip_len = network_device_read();
    if (uip_len > 0)
    {
      uip_input();
      /* If the above function invocation resulted in data that
   should be sent out on the network, the global variable
   uip_len is set to a value > 0. */
      if (uip_len > 0)
      {
        network_device_send();
      }
    }
    else if (timer_expired(&periodic_timer))
    {
      timer_reset(&periodic_timer);
      for (int i = 0; i < UIP_CONNS; i++)
      {
        uip_periodic(i);
        /* If the above function invocation resulted in data that
           should be sent out on the network, the global variable
           uip_len is set to a value > 0. */
        if (uip_len > 0)
        {
          network_device_send();
        }
      }

#if UIP_UDP
      for (int i = 0; i < UIP_UDP_CONNS; i++)
      {
        uip_udp_periodic(i);
        /* If the above function invocation resulted in data that
           should be sent out on the network, the global variable
           uip_len is set to a value > 0. */
        if (uip_len > 0)
        {
          network_device_send();
        }
      }
#endif /* UIP_UDP */
    }
  }

  while (1)
  {
    // transmit character 'A' every 1000 ms
    
    HAL_Delay(100); // Delay for 100
  }
  return 0;
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /**Initializes the CPU, AHB and APB busses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  // PeriphClkInit.AdcClockSelection    = RCC_ADCPCLK2_DIV8;  // 8 MHz
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4; // 16 MHz
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  /**Configure the Systick interrupt time
   */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

  /**Configure the Systick
   */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

extern "C" void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

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

extern "C" void DMA1_Channel2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart3_tx);
}

extern "C" void DMA1_Channel3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart3_rx);
}

extern "C" void HAL_UART_MspDeInit(UART_HandleTypeDef *uartHandle)
{
  if (uartHandle->Instance == USART3)
  {
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);
    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  }
}

extern "C" void USART3_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART2_IRQn 1 */
  if (RESET != __HAL_UART_GET_IT_SOURCE(&huart3, UART_IT_IDLE))
  {                                     // Check for IDLE line interrupt
    __HAL_UART_CLEAR_IDLEFLAG(&huart3); // Clear IDLE line flag (otherwise it will continue to enter interrupt)
                                        // usart3_rx_check();                                          // Check for data to process
  }
  /* USER CODE END USART2_IRQn 1 */
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
