/*
 * Log.cpp
 *
 *  Created on: Jul 3, 2016
 *      Author: lieven
 */
#include <log.h>
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
#include <esp_timer.h>
Log &Log::tfl(const char *lvl, const char *file, const uint32_t line)
{
    if (!txBusy)
    {
        txBusy = true;
        uint64_t msec = esp_timer_get_time() / 1000;
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
