#include "Rendering/Texture.h"

Texture::Texture(std::string path)
{
    std::string ext = boost::filesystem::path(path).extension().string();
    Image* img;
    if (ext == ".png") {
        img = ResourceManager::Load<PNG, true>(path);
		m_Type = TextureType::PNG;
    } else if (ext == ".dds") {
        img = ResourceManager::Load<DDS, true>(path);
		m_Type = TextureType::DDS;
    } else {
		m_Type = TextureType::Invalid;
        throw Resource::FailedLoadingException("Texture extension is not .png nor .dds");
    }

    this->Width = img->Width;
    this->Height = img->Height;

    GLint format;
    switch (img->Format) {
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    if (!img->Compressed) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }
    unsigned int w = img->Width;
    unsigned int h = img->Width;
    unsigned char* data = img->Data;
    for (int mipLevel = 0; mipLevel < 1 + img->MipMapLevels; mipLevel++) {
        if (img->Compressed) {
            //LOG_DEBUG("mipLevel: %i", mipLevel);
            //LOG_DEBUG("w: %i", w);
            //LOG_DEBUG("h: %i", h);

            std::size_t bpe = 16;
            std::size_t numBlocksWide = std::max<std::size_t>(1, (w + 3) / 4);
            std::size_t numBlocksHigh = std::max<std::size_t>(1, (h + 3) / 4);
            std::size_t rowBytes = numBlocksWide * bpe;
            std::size_t numRows = numBlocksHigh;
            std::size_t numBytes = rowBytes * numBlocksHigh;
            //LOG_DEBUG("numBytes: %i", numBytes);
            glCompressedTexImage2D(GL_TEXTURE_2D, mipLevel, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, w, h, 0, numBytes, data);
            data += numBytes;
        } else {
            glTexImage2D(GL_TEXTURE_2D, mipLevel, format, img->Width, img->Height, 0, format, GL_UNSIGNED_BYTE, img->Data);
        }

        w = w >> 1;
        h = h >> 1;
    }
    //LOG_DEBUG("REMAINDER: %i", img->ByteSize - (data - img->Data));
    // Generate mipmaps if the image file doesn't contain them
    if (img->MipMapLevels == 0) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    if (ext == ".png") {
        ResourceManager::Release("PNG", path);
    } else if (ext == ".dds") {
        ResourceManager::Release("DDS", path);
    }

    if (GLERROR("Texture load")) {
        throw Resource::FailedLoadingException("GL texture generation failed");
    }
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
