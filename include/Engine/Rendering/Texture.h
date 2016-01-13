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
    virtual void GlCommands() override;

    GLint m_Format;
    Image* m_Image;
public:
	~Texture();

	void Bind(GLenum textureUnit = GL_TEXTURE0);

	GLuint m_Texture = 0;
};

#endif
