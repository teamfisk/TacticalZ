#include "Rendering/Model.h"

Model::Model(std::string fileName)
{
    //Try loading the model asyncronously, if it throws any exceptions then let it propagate back to caller.
    m_RawModel = ResourceManager::Load<RawModel, true>(fileName);

    for (auto& group : m_RawModel->MaterialGroups) {
        if (!group.TexturePath.empty()) {
            group.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(group.TexturePath));
        }
        if (!group.NormalMapPath.empty()) {
            group.NormalMap = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(group.NormalMapPath));
        }
        if (!group.SpecularMapPath.empty()) {
            group.SpecularMap = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(group.SpecularMapPath));
        }
        if (!group.IncandescenceMapPath.empty()) {
            group.IncandescenceMap = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(group.IncandescenceMapPath));
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
	if (m_RawModel->isSkined()) {
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
		if (m_RawModel->isSkined()) {
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
	if (m_RawModel->isSkined()) {
		glEnableVertexAttribArray(5);
		glEnableVertexAttribArray(6);
	}
    GLERROR("GLEW: BufferFail5");

    //CreateBuffers();
}

Model::~Model()
{

}
