#include "Rendering/Model.h"

Model::Model(std::string fileName)
{
    //Try loading the model asyncronously, if it throws any exceptions then let it propagate back to caller.
    m_RawModel = ResourceManager::Load<RawModel, true>(fileName);

    for (auto& materialProperty : m_RawModel->m_Materials) {
		switch (materialProperty.type) {
		case RawModel::MaterialType::SingleTextures:
		{
			RawModel::MaterialSingleTextures* materialSingleTexture = static_cast<RawModel::MaterialSingleTextures*>(materialProperty.material);
			if (!materialSingleTexture->ColorMap.TexturePath.empty()) {
				materialSingleTexture->ColorMap.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(materialSingleTexture->ColorMap.TexturePath));
			}
			if (!materialSingleTexture->NormalMap.TexturePath.empty()) {
				materialSingleTexture->NormalMap.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(materialSingleTexture->NormalMap.TexturePath));
			}
			if (!materialSingleTexture->SpecularMap.TexturePath.empty()) {
				materialSingleTexture->SpecularMap.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(materialSingleTexture->SpecularMap.TexturePath));
			}
			if (!materialSingleTexture->IncandescenceMap.TexturePath.empty()) {
				materialSingleTexture->IncandescenceMap.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(materialSingleTexture->IncandescenceMap.TexturePath));
			}
		}
			break;
		case RawModel::MaterialType::SplatMapping:
		{
			RawModel::MaterialSplatMapping* materialSplatMapping = static_cast<RawModel::MaterialSplatMapping*>(materialProperty.material);
			if (!materialSplatMapping->SplatMap.TexturePath.empty()) {
				materialSplatMapping->SplatMap.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(materialSplatMapping->SplatMap.TexturePath));
			}
			for (auto& texture : materialSplatMapping->ColorMaps)
			{
				if (!texture.TexturePath.empty()) {
					texture.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(texture.TexturePath));
				}
				else {
					texture.Texture = nullptr;
				}
			}
			for (auto& texture : materialSplatMapping->NormalMaps)
			{
				if (!texture.TexturePath.empty()) {
					texture.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(texture.TexturePath));
				} else {
					texture.Texture = nullptr;
				}
			}
			for (auto& texture : materialSplatMapping->SpecularMaps)
			{
				if (!texture.TexturePath.empty()) {
					texture.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(texture.TexturePath));
				}
				else {
					texture.Texture = nullptr;
				}
			}
			for (auto& texture : materialSplatMapping->IncandescenceMaps)
			{
				if (!texture.TexturePath.empty()) {
					texture.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(texture.TexturePath));
				}
				else {
					texture.Texture = nullptr;
				}
			}
		}
			break;
		}
    }

    // Generate GL buffers
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

	glBufferData(GL_ARRAY_BUFFER, m_RawModel->NumVertices() * m_RawModel->VertexSize(), m_RawModel->Vertices(), GL_STATIC_DRAW);

    glGenBuffers(1, &ElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_RawModel->m_Indices.size() * sizeof(unsigned int), &m_RawModel->m_Indices[0], GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    GLERROR("GLEW: BufferFail4");

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
	std::vector<int> structSizes;
	if (m_RawModel->IsSkinned()) {
		structSizes = { 3, 3, 3, 3, 2, 4, 4 };
	} else {
		structSizes = { 3, 3, 3, 3, 2 };
	}

    int stride = 0;
    for (int size : structSizes) {
        stride += size;
    }
    stride *= sizeof(GLfloat);
    int offset = 0;
    {
        int element = 0;
        glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * offset)); element++;
        glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
        glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
        glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
        glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
		if (m_RawModel->IsSkinned()) {
			glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
			glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
		}
    }
    GLERROR("GLEW: BufferFail5");

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
	if (m_RawModel->IsSkinned()) {
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
	}
    GLERROR("GLEW: BufferFail5");

    //CreateBuffers();

    glm::vec3 mini(INFINITY);
    glm::vec3 maxi(-INFINITY);

    for (unsigned int i = 0; i < m_RawModel->NumVertices(); i++) {
        const auto& v = m_RawModel->Vertices()[i];
        mini = glm::min(mini, v.Position);
        maxi = glm::max(maxi, v.Position);
    }

    m_Box = AABB(mini, maxi);
}

Model::~Model()
{

}
