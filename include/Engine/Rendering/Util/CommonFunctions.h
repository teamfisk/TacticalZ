#ifndef CommonFuntions_h__
#define CommonFuntions_h__

#include "../../Common.h"
#include "../../OpenGL.h"
#include "../../GLM.h"
#include "../Texture.h"

namespace CommonFunctions
{ 

//Loads Texture/SpriteTexture and return null if it fails
template <typename T, bool threaded>
Texture* TryLoadResource(std::string path)
{
    Texture* img;
    try {
        img = ResourceManager::Load<T, threaded>(path);
    } catch (const Resource::StillLoadingException&) {
        img = ResourceManager::Load<T>("Textures/Core/ErrorTexture.png");
    } catch (const std::exception&) {
        img = nullptr;
    }

    return img;
}

void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);
void GenerateMultiSampleTexture(GLuint* texture, int numSamples, glm::vec2 dimensions, GLint internalFormat);
void GenerateMipMapTexture(GLuint* texture, GLenum wrapping, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type, GLint numMipMaps);
void DeleteTexture(GLuint* texture);
};

#endif