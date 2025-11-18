#ifndef SRC_LOG_H_
#define SRC_LOG_H_
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <atomic>

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

	std::atomic<bool> _busy;

public:
	Log();
	Log &tfl(const char *lvl, const char *file, const uint32_t line);
	Log &logf(const char *fmt, ...);
};

extern Log logger;
// LOG_LEVEL =>
//  0 : FATAL
//  1 : ERROR & WARN
//  2 : INFO
//  3 : DEBUG
#ifndef LOG_LEVEL
#define LOG_LEVEL 2
#endif

#define FATAL(fmt, ...) logger.tfl("F", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__);
#define ERROR(fmt, ...) logger.tfl("E", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__);
#define WARN(fmt, ...) logger.tfl("W", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__);
#define INFO(fmt, ...) logger.tfl("I", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__);
#define DEBUG(fmt, ...) logger.tfl("D", __SHORT_FILE__, __LINE__).logf(fmt, ##__VA_ARGS__);

#if LOG_LEVEL < 3
#undef DEBUG
#define DEBUG(fmt, ...) {}
#endif

#if LOG_LEVEL < 2
#undef INFO
#define INFO(fmt, ...) {}
#endif

#if LOG_LEVEL < 1
#undef ERROR
#define ERROR(fmt, ...) {}
#undef WARN
#define WARN(fmt, ...) {}
#endif

#endif /* SRC_LOG_H_ */