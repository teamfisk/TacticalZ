#ifndef BaseTexture_h__
#define BaseTexture_h__

#include <string>
#include <unordered_map>
#include <cstdio>

#include "Core/ResourceManager.h"
#include "Rendering/PNG.h"

class BaseTexture : public Resource
{
	friend class ResourceManager;

public:
	unsigned int Width = 0;
	unsigned int Height = 0;
};

#endif
