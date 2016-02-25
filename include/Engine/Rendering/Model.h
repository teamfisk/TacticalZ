#ifndef Model_h__
#define Model_h__

#include "Rendering/RawModelCustom.h"
#include "Util/CommonFunctions.h"
//#include "Rendering/RawModelAssimp.h"
#include "../OpenGL.h"
#include "Core/AABB.h"

class Model : public ThreadUnsafeResource
{
	friend class ResourceManager;

private:
	Model(std::string fileName);

public:
	~Model();
    const std::vector<RawModel::MaterialProperties>& MaterialGroups() const { return m_RawModel->m_Materials; }
    const glm::mat4& Matrix() const { return m_RawModel->m_Matrix; }

    const RawModel::RenderVertex* Vertices() const { return m_RawModel->Vertices(); }
	size_t NumberOfVertices() const { return m_RawModel->NumVertices(); }
	const std::vector<unsigned int>& Indices() const { return m_RawModel->Indices(); }

	const RawModel::Vertex* CollisionVertices() const { return m_RawModel->CollisionVertices(); }
	const std::vector<unsigned int>& CollisionIndices() const { return m_RawModel->CollisionIndices(); }

    const AABB& Box() const { return m_Box; }
	bool IsSkinned() const { return m_RawModel->IsSkinned(); }
	GLuint VAO;
	GLuint ElementBuffer;
    RawModel* m_RawModel;

private:
    AABB m_Box;

	GLuint VertexBuffer;
	GLuint NormalBuffer;
	GLuint TangentNormalsBuffer;
	GLuint BiTangentNormalsBuffer;
	GLuint TextureCoordBuffer;
};

#endif
