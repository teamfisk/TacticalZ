#ifndef Model_h__
#define Model_h__

#include "RawModelCustom.h"
#include "../OpenGL.h"

class Model : public ThreadUnsafeResource
{
	friend class ResourceManager;

private:
	Model(std::string fileName);

public:
	~Model();
    const std::vector<RawModel::MaterialGroup>& MaterialGroups() const { return m_RawModel->MaterialGroups; }
    const glm::mat4& Matrix() const { return m_RawModel->m_Matrix; }
    const std::vector<RawModel::Vertex>& Vertices() const { return m_RawModel->m_Vertices; }

	GLuint VAO;
	GLuint ElementBuffer;

private:
    RawModel* m_RawModel;
	GLuint VertexBuffer;
	GLuint NormalBuffer;
	GLuint TangentNormalsBuffer;
	GLuint BiTangentNormalsBuffer;
	GLuint TextureCoordBuffer;
};

#endif
