#ifndef Model_h__
#define Model_h__

#include "RawModelAssimp.h"
#include "../OpenGL.h"

class Model : public RawModel
{
	friend class ResourceManager;

private:
	Model(std::string fileName);

public:
	~Model();

	GLuint VAO;
	GLuint ElementBuffer;

private:
	GLuint VertexBuffer;
	GLuint DiffuseVertexColorBuffer;
	GLuint SpecularVertexColorBuffer;
	GLuint NormalBuffer;
	GLuint TangentNormalsBuffer;
	GLuint BiTangentNormalsBuffer;
	GLuint TextureCoordBuffer;
};

#endif
