#include <common.h>
extern "C"
{
#include "pico_stack.h"
#include "pico_config.h"
#include "pico_ipv4.h"
#include "pico_socket.h"
#include "pico_dev_tun.h"
#include "pico_nat.h"
#include "pico_icmp4.h"
#include "pico_dns_client.h"
#include "pico_dev_loop.h"
#include "pico_dhcp_client.h"
#include "pico_dhcp_server.h"
#include "pico_ipfilter.h"
#include "pico_dev_ppp.h"
}
#include <atomic>

#define USART3_BAUD 115200                   // UART2 baud rate (long wired cable)
#define USART3_WORDLENGTH UART_WORDLENGTH_8B // UART_WORDLENGTH_8B or UART_WORDLENGTH_9B
UART_HandleTypeDef huart3;                   // UART handle for UART3
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

u8 rx_buffer_R[RX_BUFFER_SIZE]; // Receive buffer for UART2
/*
// Example platform-specific implementation
class SLIP_Serial : public SLIP
{
public:
  // Simple interface for uIP integration
  static void slip_send(const uint8_t *packet, uint16_t length)
  {
    auto slip_packet = encode_packet(packet, length);
    // Here you would send slip_packet.data() with slip_packet.size() to your serial port
    volatile HAL_StatusTypeDef rc = HAL_UART_Transmit_DMA(&huart3, packet, length);
  }

  // Call this with incoming serial data
  static void slip_receive(const uint8_t *data, size_t length)
  {
    auto packets = process_rx_data(data, length);
    for (auto &packet : packets)
    {
      // Here you would pass the packet to uIP
      memcpy(uip_buf, packet.data(), packet.size());
      uip_len = packet.size();
      uip_input();
    }
  }
};
SLIP_Serial slip_serial; // Create an instance of the PPP_Serial class


  */

class CircBuf
{
  u8 *buffer;   // Pointer to the buffer
  u32 capacity; // Size of the buffer
  u64 head;     // Head index for writing
  u64 tail;     // Tail index for reading

public:
  CircBuf(u32 capacity) : capacity(capacity), head(0), tail(0)
  {
    buffer = new u8[capacity]; // Allocate memory for the buffer
  }

  inline u32 size() const
  {
    return head - tail; // Return the size of the buffer
  }
  // Add data to the buffer
  bool add(const u8 *data, u32 len)
  {
    while (len > 0 && size() < capacity)
    {
      buffer[head % capacity] = *data++;
      head++; // Move head index
      len--;  // Decrease the length of data to add
    }
    return true;
  }

  // Read data from the buffer
  u8 read()
  {
    if (tail == head)
    {
      return 0; // Buffer is empty
    }
    return buffer[tail++ % capacity]; // Read data and move tail index
  }
};

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
  __HAL_RCC_DMA1_CLK_ENABLE();
  // This is a simple C++ program that does nothing.
  // It serves as a placeholder for future development.
  // Set the PPP state to establish connection
}

CircBuf circ_buf(RX_BUFFER_SIZE); // Create a circular buffer with the receive buffer

void process_rx_bytes(u8 *buffer, u32 size)
{
  for (u32 i = 0; i < size; i++)
  {
    u8 byte = buffer[i];    // Get the current byte from the buffer
    circ_buf.add(&byte, 1); // Add byte to the circular buffer
  }
}

int ppp_read(struct pico_device *dev, void *buf, int len)
{
  u32 size = circ_buf.size(); // Get the size of the circular buffer
  if (size == 0)
  {
    return 0; // Return 0 if no data is available
  }
  u32 i = 0;
  for (i = 0; i < len && i < size; i++)
  {
    ((u8 *)buf)[i] = circ_buf.read(); // Read data from the circular buffer
  }
  return i; // Return the number of bytes read
}
int ppp_write(struct pico_device *dev, const void *buf, int len)
{
  // This function is called by the PPP layer to write data to the device
  // Here we can implement the logic to write data to the UART buffer

  volatile HAL_StatusTypeDef rc = HAL_UART_Transmit_DMA(&huart3, (uint8_t *)buf, len);
  if (rc != HAL_OK)
  {
    dbg("Error transmitting data: %d\n", rc);
    return -1; // Return -1 on error
  }
  return len; // Return the number of bytes written
}
int ppp_speed(struct pico_device *dev, uint32_t speed)
{
  // This function is called by the PPP layer to get the speed of the device
  // Here we can implement the logic to return the speed of the UART
  return USART3_BAUD; // Return the baud rate as the speed
}

#define NUM_PING 5 // Number of pings to send

void cb_ping(struct pico_icmp4_stats *s)
{
  char host[30];
  int time_sec = 0;
  int time_msec = 0;
  /* convert ip address from icmp4_stats structure to string */
  pico_ipv4_to_string(host, s->dst.addr);
  /* get time information from icmp4_stats structure */
  time_sec = s->time / 1000;
  time_msec = s->time % 1000;
  if (s->err == PICO_PING_ERR_REPLIED)
  {
    /* print info if no error reported in icmp4_stats structure */
    dbg("%lu bytes from %s: icmp_req=%lu ttl=%lu time=%lu ms\n",
        s->size, host, s->seq, s->ttl, s->time);
    if (s->seq >= NUM_PING)
      exit(0);
  }
  else
  {
    /* else, print error info */
    dbg("PING %lu to %s: Error %d\n", s->seq, host, s->err);
    exit(1);
  }
}

/* initialize the ping command */
void app_ping(char *dest)
{
  pico_icmp4_ping(dest, NUM_PING, 1000, 5000, 48, cb_ping);
}

/*void *pico_zalloc(int size)
{
  void *ptr = malloc(size);
  if (ptr)
  {
    memset(ptr, 0, size);
  }
  return ptr;
}
void pico_free(void *ptr)
{
  if (ptr)
  {
    free(ptr);
  }
}*/
/*
static inline unsigned long PICO_TIME(void)
{
  return HAL_GetTick()/1000; // Convert milliseconds to seconds
}
static inline unsigned long PICO_TIME_MS(void)
{
  return HAL_GetTick();
}*/

extern "C" int main()
{
  network_device_init(); // Initialize the network device
  pico_stack_init();     // Initialize the pico stack

  struct pico_device *dev = pico_ppp_create();   // Create a PPP device
  pico_ppp_set_serial_read(dev, ppp_read);       // Set the read function for PPP
  pico_ppp_set_serial_write(dev, ppp_write);     // Set the write function for PPP
  pico_ppp_set_serial_set_speed(dev, ppp_speed); // Set the speed function for PPP
  pico_ppp_connect(dev);                         // Connect the PPP device
   //  app_ping("www.google.com");                    // Start the ping application
  while (1)
  {
    pico_stack_tick(); // Call the stack tick function to process events
    HAL_Delay(1);      // Delay to prevent busy-waiting, adjust as needed
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
