#ifndef TextureSprite_h__
#define TextureSprite_h__

#include "../OpenGL.h"
#include "BaseTexture.h"
#include "Texture.h"
#include "PNG.h"

class TextureSprite : public Texture
{
	friend class ResourceManager;

protected:
	TextureSprite(std::string path);

public:
};

#endif
