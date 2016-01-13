#include "Rendering/Texture.h"

Texture::Texture(std::string path)
{
    m_Image = new PNG(path);

	if (m_Image->Width == 0 && m_Image->Height == 0 || m_Image->Format == Image::ImageFormat::Unknown) {
        delete m_Image;
		m_Image = new PNG("Textures/Core/ErrorTexture.png");
		if (m_Image->Width == 0 && m_Image->Height == 0 || m_Image->Format == Image::ImageFormat::Unknown) {
			LOG_ERROR("Couldn't even load the error texture. This is a dark day indeed.");
			return;
		}
	}

	this->Width = m_Image->Width;
	this->Height = m_Image->Height;

	switch (m_Image->Format) {
	case Image::ImageFormat::RGB:
		m_Format = GL_RGB;
		break;
	case Image::ImageFormat::RGBA:
        m_Format = GL_RGBA;
		break;
	}
}

void Texture::GlCommands()
{
	// Construct the OpenGL texture
	glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, m_Format, Width, Height, 0, m_Format, GL_UNSIGNED_BYTE, m_Image->Data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLERROR("Texture load");
    delete m_Image;
    m_Image = nullptr;
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_Texture);
}

void Texture::Bind(GLenum textureUnit /* = GL_TEXTURE0 */)
{
	glActiveTexture(textureUnit);
	glBindTexture(GL_TEXTURE_2D, m_Texture);
}
