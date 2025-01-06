/*
 * log.h
 *
 *  Created on: Nov 7, 2021
 *      Author: lieven
 */

#ifndef SRC_LOG_H_
#define SRC_LOG_H_
#include <stdint.h>
#include <stdarg.h>
#include <string>
#include <string.h>
#include <errno.h>

#define ColorOrange "\033[33m"
#define ColorGreen "\033[32m"
#define ColorPurple "\033[35m"
#define ColorDefault "\033[39m"

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
	bool txBusy = false;
	Log &tfl(const char *lvl, const char *file, const uint32_t line);
	Log &logf(const char *fmt, ...);
	void flush();
	void setLevel(Level);

private:
};

extern Log logger;

#define INFO(fmt, ...)                                                                  \
	{                                                                                   \
		if (logger._level <= Log::L_INFO )                                               \
			logger.tfl("I", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
	}
#define WARN(fmt, ...)                                                                  \
	{                                                                                   \
		if (logger._level <= Log::L_WARN  )                                               \
			logger.tfl("W", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
	}
#define DEBUG(fmt, ...)                                                                 \
	{                                                                                   \
		if (logger._level <= Log::L_DEBUG )                                              \
			logger.tfl("D", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
	}
#define ERROR(fmt, ...)                                                                 \
	{                                                                                   \
		if (logger._level <= Log::L_ERROR )                                              \
			logger.tfl("E", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__).flush(); \
	}

#endif /* SRC_LOG_H_ */