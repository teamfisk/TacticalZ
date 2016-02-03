#ifndef GLError_h__
#define GLError_h__

#include <iostream>
#include "Common.h"

inline bool _GLERROR(const char* info, const char* file, const char* func, unsigned int line)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		_LOG(LOG_LEVEL_ERROR, file, func, line, "GL Error: %s\nError code: %i, %s\n", info, error, gluErrorString(error));
		return true;
	}

	return false;
}

#define GLERROR(function) \
	_GLERROR(function, __BASE_FILE__, __func__, __LINE__)

#endif