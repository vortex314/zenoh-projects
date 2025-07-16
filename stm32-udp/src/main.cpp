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
#include "pico_dev_slip.h"
}
#include <atomic>

#define USART3_BAUD 115200                   // UART2 baud rate (long wired cable)
#define USART3_WORDLENGTH UART_WORDLENGTH_8B // UART_WORDLENGTH_8B or UART_WORDLENGTH_9B
UART_HandleTypeDef huart3;                   // UART handle for UART3
DMA_HandleTypeDef hdma_usart3_rx;
DMA_HandleTypeDef hdma_usart3_tx;

u8 rx_buffer_R[RX_BUFFER_SIZE]; // Receive buffer for UART2

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

CircBuf circ_buf(RX_BUFFER_SIZE); // Create a circular buffer with the receive buffer

void udp_init();
void udp_send();

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

int BSP_serial_init(uint32_t base, uint32_t baudrate)
{
  return 0;
}
int BSP_serial_write(uint32_t base, const uint8_t *data, uint32_t len)
{
  volatile HAL_StatusTypeDef rc = HAL_UART_Transmit_DMA(&huart3, data, len);
  if (rc != HAL_OK)
  {
    dbg("Error transmitting data: %d\n", rc);
    return -1; // Return -1 on error
  }
  return len; // Return the number of bytes written
}
int BSP_serial_read(uint32_t base, uint8_t *data, uint32_t len)
{
  uint32_t offset = 0;

  while (offset < len)
  {
    if (circ_buf.size() > 0)
    {
      data[offset++] = circ_buf.read(); // Read data from the circular buffer
    }
    else
    {
      break; // Exit if no more data is available
    }
  }
  return offset; // Return the number of bytes read
}

void process_rx_bytes(u8 *buffer, u32 size)
{
  for (u32 i = 0; i < size; i++)
  {
    u8 byte = buffer[i];    // Get the current byte from the buffer
    circ_buf.add(&byte, 1); // Add byte to the circular buffer
  }
}

extern "C" int main()
{
  network_device_init(); // Initialize the network device
  pico_stack_init();     // Initialize the pico stack
  struct pico_ip4 local_addr;
  struct pico_ip4 gtw_addr;
  /* Assign IP address to device */
  pico_string_to_ipv4("192.168.1.2", &local_addr.addr);
  pico_string_to_ipv4("192.168.1.1", &gtw_addr.addr); // Set gateway address

  // Initialize the SLIP device with the name "slip0", base address 0x40004800, and baud rate 115200
  struct pico_device *slip_dev = pico_slip_create("slip0", 0x40004800, 115200);
  if (slip_dev == NULL)
  {
    dbg("Failed to create SLIP device\n");
    return -1; // Return -1 if SLIP device creation fails
  }
  // Set the SLIP device's send and poll functions
  struct pico_ip4 netmask;
  pico_string_to_ipv4("255.255.255.0", &netmask.addr); // Subnet mask (/24)
  pico_ipv4_link_add(slip_dev, local_addr, netmask); // Add SLIP device link with local address and netmask

  struct pico_ip4 gateway, dest, netmask_all;
  pico_string_to_ipv4("192.168.1.1", &gateway.addr); // Gateway IP
  pico_string_to_ipv4("0.0.0.0", &dest.addr);        // Destination (0.0.0.0 for default route)
  pico_string_to_ipv4("0.0.0.0", &netmask_all.addr);     // Netmask (0.0.0.0 for default route)

  struct pico_ipv4_link *link = pico_ipv4_link_get(&local_addr); // Get link for local IP
  if (link)
  {
    pico_ipv4_route_add(dest, netmask_all, gateway, 1, link); // Metric set to 1
  }
  else
  {
    return -1;
  }

  //  app_ping("www.google.com");                    // Start the ping application
  uint32_t counter = 0;
  /* Create UDP socket */
  struct pico_socket *sock;
  uint16_t local_port = 12345;
  uint16_t remote_port = 54321;
  struct pico_ip4 remote_addr;
  char *data = "{\"type\":\"udp\",\"data\":\"Hello, UDP!\"}"; // Data to send in the UDP packet

  sock = pico_socket_open(PICO_PROTO_IPV4, PICO_PROTO_UDP, NULL);
  if (!sock)
  {
    pico_device_destroy(slip_dev);
    return -1;
  }

  /* Bind socket to local address and port */
  int ret = pico_socket_bind(sock, &local_addr, &local_port);
  if (ret < 0)
  {
    pico_socket_close(sock);
    pico_device_destroy(slip_dev);
    return -1;
  }

  /* Set destination address */
  pico_string_to_ipv4("192.168.1.1", &remote_addr.addr);
  while (1)
  {
    pico_stack_tick(); // Call the stack tick function to process events
    counter++;
    if (counter % 1000 == 0)
    {
      /* Send UDP packet */
      ret = pico_socket_sendto(sock, data, strlen(data), &remote_addr, remote_port);
      if (ret < 0)
      {
        pico_socket_close(sock);
        pico_device_destroy(slip_dev);
        return -1;
      }
    } // Send UDP packets every 1000 ticks
    HAL_Delay(1); // Delay to prevent busy-waiting, adjust as needed
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
