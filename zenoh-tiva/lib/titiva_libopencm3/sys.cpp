#include <sys.h>
#include <cassert>
#include <util.h>
#include <stdarg.h>


// general failure -> stall
extern "C" void panic_handler(const char *msg)
{
    volatile const char* m = msg;
    (void)m;
    while (1)
    {
    }
}

void SystemClock_Config(void);
// replace weak systick handler
static uint64_t __tick_count=0;

extern "C" void SysTick_Handler(void)
{
    
    __tick_count++;
}

uint64_t millis()
{
    return __tick_count;
}


void sys_init()
{

   
    // Configure the system clock
    SystemClock_Config();
}

void delay(size_t msec)
{
    volatile uint64_t start = millis();
    while (millis()  < start + msec)
    {
    }
}
void delayMicroseconds(size_t usec)
{
    PANIC("delayMicroseconds not implemented");
    (void)usec;
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
    uint64_t msec = 0;
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

#include <libopencm3/lm4f/systemcontrol.h>
#include <libopencm3/lm4f/gpio.h>
#include <libopencm3/lm4f/uart.h>
#include <libopencm3/lm4f/nvic.h>

//==================================================================
void SystemClock_Config(void)
{
    gpio_enable_ahb_aperture();
}

void *__dso_handle = 0;

