#ifndef CommonFuntions_h__
#define CommonFuntions_h__

#include "../../Common.h"
#include "../../OpenGL.h"
#include "../../GLM.h"
#include "../Texture.h"

namespace CommonFunctions
{ 
Texture* LoadTexture(std::string path, bool threaded);
void GenerateTexture(GLuint* texture, GLenum wrapping, GLenum filtering, glm::vec2 dimensions, GLint internalFormat, GLint format, GLenum type);
void GenerateMultiSampleTexture(GLuint* texture, int numSamples, glm::vec2 dimensions, GLint internalFormat);
void GenerateMipMapTexture(GLuint* texture, GLenum wrapping, glm::vec2 dimensions, GLint format, GLenum type, GLint numMipMaps);
void DeleteTexture(GLuint* texture);
};

#endif