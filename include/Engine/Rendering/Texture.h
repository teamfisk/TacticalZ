#ifndef Texture_h__
#define Texture_h__

#include "../OpenGL.h"
#include "BaseTexture.h"
#include "PNG.h"

class Texture : public BaseTexture
{
	friend class ResourceManager;

private:
	Texture(std::string path);

public:
	~Texture();

	void Bind(GLenum textureUnit = GL_TEXTURE0);

	GLuint m_Texture = 0;
    
};

#endif
