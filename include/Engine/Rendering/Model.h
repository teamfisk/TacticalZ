#ifndef Model_h__
#define Model_h__

#include "Rendering/RawModelCustom.h"
#include "Util/CommonFunctions.h"
//#include "Rendering/RawModelAssimp.h"
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
    RawModel* m_RawModel;

private:
    
	GLuint VertexBuffer;
	GLuint NormalBuffer;
	GLuint TangentNormalsBuffer;
	GLuint BiTangentNormalsBuffer;
	GLuint TextureCoordBuffer;
};

#endif
