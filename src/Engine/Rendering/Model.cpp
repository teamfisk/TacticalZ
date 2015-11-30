#include "Rendering/Model.h"

Model::Model(std::string fileName)
	: RawModel(fileName)
{
	// Generate GL buffers
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), &m_Vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &ElementBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ElementBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(unsigned int), &m_Indices[0], GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	GLERROR("GLEW: BufferFail4");

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	std::vector<int> structSizes = { 3, 3, 3, 3, 2, 4, 4, 4, 4, 4, 4 };
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
		glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
		glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
		glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
		glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
		glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
		glVertexAttribPointer(element, structSizes[element], GL_FLOAT, GL_FALSE, stride, (GLvoid*)(sizeof(GLfloat) * (offset += structSizes[element - 1]))); element++;
	}
	GLERROR("GLEW: BufferFail5");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);
	glEnableVertexAttribArray(9);
	glEnableVertexAttribArray(10);
	GLERROR("GLEW: BufferFail5");

	//CreateBuffers();
}

Model::~Model()
{

}
