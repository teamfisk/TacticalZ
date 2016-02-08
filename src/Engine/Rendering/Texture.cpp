#include "Rendering/Texture.h"

Texture::Texture(std::string path)
{

    PNG image(path);

    if (image.Width == 0 && image.Height == 0 || image.Format == Image::ImageFormat::Unknown) {
        //image = PNG("Textures/Core/ErrorTexture.png");
        return; // Temporary fix to remove crash
        /*
        if (image.Width == 0 && image.Height == 0 || image.Format == Image::ImageFormat::Unknown) {
            LOG_ERROR("Couldn't even load the error texture. This is a dark day indeed.");
            return;
        }*/
    }

    this->Width = image.Width;
    this->Height = image.Height;

    GLint format;
    switch (image.Format) {
    case Image::ImageFormat::RGB:
        format = GL_RGB;
        break;
    case Image::ImageFormat::RGBA:
        format = GL_RGBA;
        break;
    }

    // Construct the OpenGL texture
    glGenTextures(1, &m_Texture);
    glBindTexture(GL_TEXTURE_2D, m_Texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, image.Width, image.Height, 0, format, GL_UNSIGNED_BYTE, image.Data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GLERROR("Texture load");
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
