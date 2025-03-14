#include <sys.h>
#include <cassert>

HardwareSerial::HardwareSerial(int) {}
int HardwareSerial::begin(uint32_t baudrate)
{
    assert(false);
    return -1;
}
int HardwareSerial::end()
{
    assert(false);
    return -1;
}
int HardwareSerial::flush() { return -1; }
int HardwareSerial::write(uint8_t *, size_t) { return -1; }
int HardwareSerial::available() { return -1; }
uint8_t HardwareSerial::read() { return -1; }

void delay(size_t msec)
{
}
void delayMicroseconds(size_t usec)
{
}
uint64_t millis()
{
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
    if (txBusy)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
    return *this;
}

void Log::setLevel(Level level)
{
    _level = level;
}

void Log::flush()
{
    printf("\n");
    txBusy = false;
}
#include <sys/time.h>

Log &Log::tfl(const char *lvl, const char *file, const uint32_t line)
{
    if (!txBusy)
    {
        txBusy = true;
        uint64_t msec = HAL_getTick() / 1000;
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
    }
    return *this;
}


