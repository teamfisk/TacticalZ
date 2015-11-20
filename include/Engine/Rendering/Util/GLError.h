/*
	This file is part of Daydream Engine.
	Copyright 2014 Adam Byléhn, Tobias Dahl, Simon Holmberg, Viktor Ljung
	
	Daydream Engine is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	Daydream Engine is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with Daydream Engine.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GLError_h__
#define GLError_h__

#include "PrecompiledHeader.h"
#include <iostream>

inline bool _GLERROR(const char* info, const char* file, const char* func, unsigned int line)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		_LOG(LOG_LEVEL_ERROR, file, func, line, "GL Error: %s %i %s", info, error, gluErrorString(error));
		return true;
	}

	return false;
}

#define GLERROR(function) \
	_GLERROR(function, __BASE_FILE__, __func__, __LINE__)

#endif // GLError_h__