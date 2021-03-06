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
    const std::vector<RawModel::MaterialProperties>& MaterialGroups() const { return m_Materials; }
	unsigned int NumberOfVertices() const { return m_Vertices.size(); }

    const AABB& Box() const { return m_Box; }
	bool IsSkinned() const { return m_IsSkinned; }
	GLuint VAO;
	GLuint ElementBuffer;
    //RawModel* m_RawModel;

    Skeleton* m_Skeleton = nullptr;
    std::vector<glm::vec3> m_Vertices;
    std::vector<unsigned int> m_Indices;

private:
    AABB m_Box;

	GLuint VertexBuffer;
	GLuint NormalBuffer;
	GLuint TangentNormalsBuffer;
	GLuint BiTangentNormalsBuffer;
	GLuint TextureCoordBuffer;

	std::vector<RawModel::MaterialProperties> m_Materials;
    bool m_IsSkinned;
};

#endif
