#ifndef Logging_h__
#define Logging_h__

#ifdef _WIN32
// http://stackoverflow.com/a/2282433
#define __func__ __FUNCTION__
// http://stackoverflow.com/a/8488201
#define __BASE_FILE__ \
	(strrchr(__FILE__, '/') \
	? strrchr(__FILE__, '/') + 1 \
	: (strrchr(__FILE__, '\\') \
	? strrchr(__FILE__, '\\') + 1 \
	: __FILE__))
#endif

#include <iostream>
#include <limits>
#include <string>
#include <cstdio>
#include <stdarg.h>

enum _LOG_LEVEL
{
	LOG_LEVEL_ERROR,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_DEBUG
};

extern _LOG_LEVEL LOG_LEVEL;

extern const char* _LOG_LEVEL_PREFIX[];

extern void _LOG(_LOG_LEVEL logLevel, const char* file, const char* func, unsigned int line, const char* format, ...);

#define LOG(logLevel, format, ...) \
	_LOG(logLevel, __BASE_FILE__, __func__, __LINE__, format, ##__VA_ARGS__)

#define LOG_ERROR(format, ...) \
	LOG(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

#define LOG_WARNING(format, ...) \
	LOG(LOG_LEVEL_WARNING, format, ##__VA_ARGS__)

#define LOG_INFO(format, ...) \
	LOG(LOG_LEVEL_INFO, format, ##__VA_ARGS__)

#ifdef DEBUG
#define LOG_DEBUG(format, ...) \
	LOG(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...) 
#endif

#endif // Logging_h__