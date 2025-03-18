#ifndef _SYS_H_
#define _SYS_H_
#include <errno.h>
#include <stdint.h>
#include <stm32f401xe.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_adc.h>
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_usart.h"
#include <util.h>

extern "C" void panic_handler(const char *msg);
Res<int> sys_init();

void delay(size_t msec);
void delayMicroseconds(size_t usec);
uint64_t millis();

class Log
{

public:
    typedef enum
    {
        L_DEBUG,
        L_INFO,
        L_WARN,
        L_ERROR
    } Level;
    Level _level = L_INFO;

    Log();
    Log &tfl(const char *lvl, const char *file, const uint32_t line);
    Log &logf(const char *fmt, ...);
    void flush();
    void setLevel(Level);

private:
};
using cstr = const char *const;

static constexpr const char *past_last_slash(cstr str, cstr last_slash)
{
    return *str == '\0' ? last_slash : *str == '/' ? past_last_slash(str + 1, str + 1)
                                                   : past_last_slash(str + 1, last_slash);
}

static constexpr const char *past_last_slash(cstr str)
{
    return past_last_slash(str, str);
}

#define __SHORT_FILE__                                      \
    (                                                       \
        {                                                   \
            constexpr cstr sf__{past_last_slash(__FILE__)}; \
            sf__;                                           \
        })
extern Log logger;

#if ZENOH_DEBUG==3
#define INFO(fmt, ...)                                                                  \
    {                                                                                   \
        if (logger._level <= Log::L_INFO)                                               \
            logger.tfl("I", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
    }
#define WARN(fmt, ...)                                                                  \
    {                                                                                   \
        if (logger._level <= Log::L_WARN)                                               \
            logger.tfl("W", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
    }
#define DEBUG(fmt, ...)                                                                 \
    {                                                                                   \
        if (logger._level <= Log::L_DEBUG)                                              \
            logger.tfl("D", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
    }
#define ERROR(fmt, ...)                                                                 \
    {                                                                                   \
        if (logger._level <= Log::L_ERROR)                                              \
            logger.tfl("E", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
    }
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define PANIC(msg) { ERROR("PANIC: " msg); panic_handler(msg); }
#else
#define INFO(fmt, ...) {}
#define WARN(fmt, ...) {}
#define DEBUG(fmt, ...) {}
#define ERROR(fmt, ...)  {}
#define PANIC(msg) { panic_handler("fatal ");}
#endif

#endif