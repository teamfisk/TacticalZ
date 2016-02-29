#include "Rendering/Util/CommonFunctions.h"

Texture* CommonFunctions::LoadTexture(std::string path, bool threaded)
{
    Texture* img;
    try {
        if(threaded) {
            img = ResourceManager::Load<Texture, true>(path);
        } else {
            img = ResourceManager::Load<Texture, false>(path);
        }
    } catch (const Resource::StillLoadingException&) {
        img = ResourceManager::Load<Texture>("Textures/Core/ErrorTexture.png");
    } catch (const std::exception&) {
        img = nullptr;
    }

    return img;
}

void CommonFunctions::GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type)
{
	glDeleteTextures(1, texture);
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, dimensions.x, dimensions.y, 0, format, type, nullptr);
	GLERROR("Texture initialization failed");
}

void CommonFunctions::GenerateMultiSampleTexture(GLuint* texture, int numSamples, glm::vec2 dimensions, GLint internalFormat)
{
	glDeleteTextures(1, texture);
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, *texture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, numSamples, internalFormat, dimensions.x, dimensions.y, false);
	GLERROR("Texture initialization failed");
}


void CommonFunctions::GenerateMipMapTexture(GLuint* texture, GLenum wrapping, glm::vec2 dimensions, GLint format, GLenum type, GLint numMipMaps)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);
	glTexStorage2D(GL_TEXTURE_2D, numMipMaps, GL_RGBA8, dimensions.x, dimensions.y);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, dimensions.x, dimensions.y, format, type, texture);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLERROR("MipMap Texture initialization failed");
}

void CommonFunctions::DeleteTexture(GLuint* texture) 
{
	glDeleteTextures(1, texture);
	*texture = 0;
}