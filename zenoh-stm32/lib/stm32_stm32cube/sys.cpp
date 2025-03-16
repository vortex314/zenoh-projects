#include <sys.h>
#include <cassert>
#include <util.h>
#include <stdarg.h>

#include <stm32f4xx.h>
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_usart.h"

#include <Serial.h>


// redirect printf to UART2
extern "C" int _write(int file, char *ptr, int len)
{
    Serial2.write((uint8_t *)ptr, len);
    return len;
}
// general failure -> stall
extern "C" void panic_handler(const char *msg)
{
    __disable_irq();
    while (1)
    {
    }
}

void GPIO_Config(USART_TypeDef *usart);
void SystemClock_Config(void);


// replace weak systick handler
extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
}

Res<int> sys_init()
{
    Res<int> res;
    HAL_Init();
    // Configure the system clock
    SystemClock_Config();
    // Configure the GPIO
 /*   GPIO_Config(USART1);
    GPIO_Config(USART2);
    // Configure the USART
    USART_Config(USART1).and_then([&](UART_HandleTypeDef *uart)
                                  { huart1 = uart; });
    // Start UART reception in interrupt mode
    USART_Config(USART2).and_then([&](UART_HandleTypeDef *uart)
                                  { huart2 = uart; });
    Serial1 = new HardwareSerial(1);
    Serial2 = new HardwareSerial(2);*/
    return res.ok(0);
}
/*
HardwareSerial::HardwareSerial(int uart)
{
    if (uart == 1)
    {
        huart = huart1;
        auto r = HAL_UART_Receive_IT(huart1, rx_buffer, 1);
        if (r != HAL_OK)
        {
            PANIC("HAL_UART_Receive_IT failed");
        }
    }
    else if (uart == 2)
    {
        huart = huart2;
        auto r = HAL_UART_Receive_IT(huart2, rx_buffer, 1);
        if (r != HAL_OK)
        {
            PANIC("HAL_UART_Receive_IT failed");
        }
    }
    else
    {
        PANIC("Invalid UART device");
    }
    rx_data.reserve(256);
}

int HardwareSerial::begin(uint32_t baudrate)
{
    INFO("baudrate set %u", baudrate);
    return 0;
}
int HardwareSerial::end()
{
    INFO("HardwareSerial::end");
    return 0;
}
int HardwareSerial::flush() { return -1; }
*/

/*
int HardwareSerial::write(uint8_t *bytes, size_t length)
{
    INFO("HardwareSerial::write [%lu] [ %s]", length, bytes_to_hex(bytes,length).c_str());
    return (int)HAL_UART_Transmit(huart1, bytes, length, HAL_MAX_DELAY);
}

int HardwareSerial::available() { return rx_data.size(); }
uint8_t HardwareSerial::read() { return -1; }
*/
void delay(size_t msec)
{
    uint64_t start = millis();
    while (millis() - start < msec)
    {
    }
}
void delayMicroseconds(size_t usec)
{
    PANIC("delayMicroseconds not implemented");
}
uint64_t millis()
{
    static uint64_t ms = 0;
    uint64_t t = HAL_GetTick();
    if (t < ms)
    {
        ms += 0x100000000;
    }
    return ms + t;
}

#include <sys.h>
extern "C"
{
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
}
Log::Log()
{
    _level = Level::L_INFO;
    txBusy = false;
}
// log the file and line
Log &Log::logf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    return *this;
}

void Log::setLevel(Level level)
{
    _level = level;
}

#include <sys/time.h>

Log &Log::tfl(const char *lvl, const char *file, const uint32_t line)
{
    uint64_t msec = HAL_GetTick();
    uint32_t sec = msec / 1000;
    uint32_t min = sec / 60;
    uint32_t hr = min / 60;
    uint32_t ms = msec % 1000;
    printf("%s %2.2d:%2.2d:%2.2d.%3.3d | %15.15s:%4u | ", lvl,
           (int)hr % 24,
           (int)min % 60,
           (int)sec % 60,
           (int)ms,
           file, (unsigned int)line);
    return *this;
}

void Log::flush()
{
    printf("\r\n");
    txBusy = false;
}

//==================================================================
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the main internal regulator output voltage
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
    // Initializes the RCC Oscillators according to the specified parameters
    // in the RCC_OscInitTypeDef structure.

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        PANIC("HAL_RCC_OscConfig failed");
    }
    // Initializes the CPU, AHB and APB buses clocks

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
    {
        PANIC("HAL_RCC_ClockConfig failed");
    }
}
/*
Res<UART_HandleTypeDef *> USART_Config(USART_TypeDef *usart)
{
    // Enable USART2 clock
    UART_HandleTypeDef *huart = (UART_HandleTypeDef *)malloc(sizeof(UART_HandleTypeDef));
    Res<UART_HandleTypeDef *> res;
    if (usart == USART2)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
    }
    if (usart == USART1)
    {
    }
    // Configure USART2
    huart->Instance = usart;
    huart->Init.BaudRate = 115200;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_StatusTypeDef r = HAL_UART_Init(huart);
    if (r != HAL_OK)
    {
        free(huart);
        return res.err(Error{r, "HAL_UART_Init failed"});
    }
    return res.ok(huart);
}

void GPIO_Config(USART_TypeDef *usart)
{

    if (usart == USART2)
    {
        // USART2 GPIO Configuration
        // PA2     ------> USART2_TX
        // PA3     ------> USART2_RX
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        // Enable GPIOA clock
        __HAL_RCC_GPIOA_CLK_ENABLE();
        // Configure PA.2 (USART2_TX), PA.3 (USART2_RX)
        GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
    if (usart == USART1)
    {

    }
}

int HardwareSerial::rx_isr()
{
    rx_data.push_back(rx_buffer[0]);
    if (rx_data.size() > 255)
    {
        rx_data.clear();
    }
    HAL_UART_Transmit(huart1, Serial1->rx_buffer, 1, HAL_MAX_DELAY);
    // Restart UART reception in interrupt mode
    HAL_UART_Receive_IT(huart1, Serial1->rx_buffer, 1);
    return 0;
}

// UART Receive Complete Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        Serial1->rx_isr();
    }
}

extern "C" void USART1_IRQHandler(void)
{
    if (huart1 != NULL)
        HAL_UART_IRQHandler(huart1);
}

extern "C" void USART2_IRQHandler(void)
{
    if (huart2 != NULL)
        HAL_UART_IRQHandler(huart2);
}
        */