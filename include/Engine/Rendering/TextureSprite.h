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
	~TextureSprite();

	void Bind(GLenum textureUnit = GL_TEXTURE0);

	GLuint m_Texture = 0;
    unsigned char* Data = nullptr;
    
};

#endif
