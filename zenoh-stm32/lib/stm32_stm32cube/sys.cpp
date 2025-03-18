#include <sys.h>
#include <cassert>
#include <util.h>
#include <stdarg.h>

#include <stm32f4xx.h>
#include "stm32f4xx_hal_rcc.h"

// general failure -> stall
extern "C" void panic_handler(const char *msg)
{
    volatile const char* m = msg;
    __disable_irq();
    while (1)
    {
    }
}

void SystemClock_Config(void);
// replace weak systick handler
static uint64_t __tick_count=0;

extern "C" void SysTick_Handler(void)
{
    HAL_IncTick();
    __tick_count++;
}

uint64_t millis()
{
    return __tick_count;
}


Res<int> sys_init()
{
    Res<int> res;
    HAL_Init();
    // Configure the system clock
    SystemClock_Config();
    return res.ok(0);
}

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
