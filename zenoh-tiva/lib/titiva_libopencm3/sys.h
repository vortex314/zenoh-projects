#ifndef _SYS_H_
#define _SYS_H_
#include <errno.h>
#include <stdint.h>
#include <util.h>
#include <printf.h>

#include <libopencm3/cm3/systick.h>

extern "C" void panic_handler(const char *msg);
void sys_init();

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
    static void time(char *buf, unsigned long buflen);

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
#define ERROR(fmt, ...)                                                                 \
    {                                                                                   \
        if (logger._level <= Log::L_ERROR)                                              \
            logger.tfl("E", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
    }
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#undef PANIC
#define PANIC(msg)            \
    {                         \
        ERROR("PANIC: " msg); \
        panic_handler(msg);   \
    }
#if ZENOH_DEBUG == 3
#define DEBUG(fmt, ...)                                                                 \
    {                                                                                   \
        if (logger._level <= Log::L_DEBUG)                                              \
            logger.tfl("D", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
    }

#else
#define DEBUG(fmt, ...) \
    {                   \
    }
#endif
#define PANIC(msg)               \
    {                            \
        panic_handler("fatal "); \
    }

#endif