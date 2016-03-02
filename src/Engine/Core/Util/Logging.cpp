#include "Core/Util/Logging.h"

#ifdef DEBUG
_LOG_LEVEL LOG_LEVEL = LOG_LEVEL_DEBUG;
#else
_LOG_LEVEL LOG_LEVEL = LOG_LEVEL_INFO;
#endif

const char* _LOG_LEVEL_PREFIX[] =
{
    "EE: ",
	"",
	"WW: ",
	"DD: "
};

void _LOG(_LOG_LEVEL logLevel, const char* file, const char* func, unsigned int line, const char* format, ...)
{
	if (logLevel > LOG_LEVEL) {
		return;
	}

	char* message = nullptr;
	va_list args;

	va_start(args, format);
	size_t size = vsnprintf(message, 0, format, args) + 1;
	va_end(args);

	va_start(args, format);
	message = new char[size];
	vsnprintf(message, size, format, args);
	va_end(args);

	if (logLevel == LOG_LEVEL_ERROR) {
		/*std::cerr << file << ":" << line << " " << func << std::endl;
		std::cerr << _LOG_LEVEL_PREFIX[logLevel] << message << std::endl;*/
	} else {
		std::cout << _LOG_LEVEL_PREFIX[logLevel] << message << std::endl;
	}

	delete[] message;
}