#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#include <string.h>
#include <stm32f1xx_hal.h>        // Include the HAL library for STM32F1 series
#include <stm32f1xx_hal_def.h>    // Include the HAL definitions for STM32F1 series
#include <stm32f1xx_hal_cortex.h> // Include the HAL Cortex functions for STM32F1 series
#include <stm32f1xx_hal_rcc.h>    // Include the HAL RCC functions for STM32F1 series
#include <stm32f1xx_hal_uart.h>
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

extern "C"
{

}

extern void process_rx_bytes(u8 *buffer, u32 size); // Create an instance of the PPP_Serial class
extern void usart3_rx_check(void); // Function prototype for checking received data in UART3
extern volatile bool ppp_frame_ready;

void UART3_Init(void); // Function prototype for UART2 initialization
void SystemClock_Config(void);
void UART_DisableRxErrors(UART_HandleTypeDef *huart);
#define USART3_BAUD 115200                   // UART2 baud rate (long wired cable)
#define USART3_WORDLENGTH UART_WORDLENGTH_8B // UART_WORDLENGTH_8B or UART_WORDLENGTH_9B
extern UART_HandleTypeDef huart3;            // UART handle for UART3
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;

#define RX_BUFFER_SIZE 1024                            // Define the size of the receive buffer
extern u8 rx_buffer_R[RX_BUFFER_SIZE];               // Receive buffer for UART2
constexpr u32 rx_buffer_R_len = sizeof(rx_buffer_R); // Length of the receive buffer
#endif