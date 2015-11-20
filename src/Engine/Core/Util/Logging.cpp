#include "Core/Util/Logging.h"

#ifdef DEBUG
_LOG_LEVEL LOG_LEVEL = LOG_LEVEL_DEBUG;
#else
_LOG_LEVEL LOG_LEVEL = LOG_LEVEL_INFO;
#endif