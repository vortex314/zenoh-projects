#include <common.h>

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
u8 rx_buffer_R[RX_BUFFER_SIZE]; // Receive buffer for UART2


extern "C" void uip_callback() {
  while(1){

  }

}

extern "C" void uip_log(char *s) {}

// Example platform-specific implementation
class PPP_Serial : public PPP
{
public:
  void serial_send(const uint8_t *data, uint16_t len) override
  {
    volatile HAL_StatusTypeDef rc = HAL_UART_Transmit_DMA(&huart3, data, len);
  }

  void uip_input_packet(const uint8_t *packet, uint16_t len) override
  {
    // Forward packet to uIP stack
    memcpy(uip_buf, packet, len);
    uip_len = len;
    uip_input();
  }
};
PPP_Serial ppp_serial; // Create an instance of the PPP_Serial class

void process_rx_bytes(u8 *buffer, u32 size)
{
  // Process the received bytes from the PPP instance
  for (u32 i = 0; i < size; i++)
  {
    ppp_serial.process_byte(buffer[i]);
  }
}

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
  volatile HAL_StatusTypeDef rc = HAL_UART_Receive_DMA(&huart3, (uint8_t *)rx_buffer_R, sizeof(rx_buffer_R));
  UART_DisableRxErrors(&huart3);
  __HAL_RCC_DMA1_CLK_ENABLE();
  // This is a simple C++ program that does nothing.
  // It serves as a placeholder for future development.
  ppp_serial.init(); // Initialize the PPP instance
  // Set the PPP state to establish connection
}


extern "C" int main()
{
  struct V {
    uip_ipaddr_t ipaddr;
    uip_ipaddr_t netmask;
    uip_ipaddr_t gw;
  } v;
  struct V *vp = new struct V;
  uip_ipaddr_t ipaddr;
  struct timer periodic_timer;
  timer_set(&periodic_timer, CLOCK_SECOND / 2);
  uip_ipaddr(ipaddr, 192, 168, 0, 2);
  uip_sethostaddr(ipaddr);

  network_device_init(); // Initialize the network device
  uip_init();

  while (1)
  {
    ppp_serial.tick(HAL_GetTick()); // Call the PPP tick function to handle timeouts and retransmissions

    /*uip_len = network_device_read();
    if (uip_len > 0)
    {
      uip_input();

      if (uip_len > 0)
      {
  ppp_serial.send_ip_packet(uip_buf, uip_len);
      }
    }
    else*/
    if (timer_expired(&periodic_timer))
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
          ppp_serial.send_ip_packet(uip_buf, uip_len);
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
          ppp_serial.send_ip_packet(uip_buf, uip_len);
        }
      }
#endif /* UIP_UDP */
    }
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
