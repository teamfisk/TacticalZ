#include "Rendering\RawModelCustom.h"

RawModel::RawModel(std::string fileName)
{
    char* fileData;
    std::ifstream in(fileName.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
            throw Resource::FailedLoadingException("Open file failed");
    }
    unsigned int fileByteSize = in.tellg();
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    unsigned int offset = 0;
    if (fileByteSize > 0) {
        ReadMeshFileHeader(offset, fileData, fileByteSize);
        ReadMesh(offset, fileData, fileByteSize);
    }
    delete fileData;
    MaterialGroup mat;
    mat.StartIndex = 0;
    mat.EndIndex = m_Indices.size() - 1;
    MaterialGroups.push_back(mat);
}

void RawModel::ReadMeshFileHeader(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    unsigned int test;
    test = *(unsigned int*)fileData;
    unsigned int* test2;
    test2 = (unsigned int*)fileData;

    m_Vertices.resize(test);
    offset += sizeof(unsigned int);
    test = *(unsigned int*)(fileData + offset);
    m_Indices.resize(*(unsigned int*)(fileData + offset));
    offset += sizeof(unsigned int);
#else
#endif
}

void RawModel::ReadMesh(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
    ReadVertices(offset, fileData, fileByteSize);
    ReadIndices(offset, fileData, fileByteSize);
}

void RawModel::ReadVertices(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    if (offset +  m_Vertices.size() * sizeof(Vertex) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading vertices failed");
    }
    unsigned int i = sizeof(Vertex);
    unsigned int ii = sizeof(unsigned int);
    memcpy(&m_Vertices[0], fileData + offset, m_Vertices.size() * sizeof(Vertex));
    offset += m_Vertices.size() * sizeof(Vertex);
#else
#endif
}

void RawModel::ReadIndices(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    if (offset + m_Indices.size() * sizeof(unsigned int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading indices failed");
    }

    memcpy(&m_Indices[0], fileData + offset, m_Indices.size() * sizeof(unsigned int));
    offset += m_Indices.size() * sizeof(unsigned int);

#else
#endif
}


RawModel::~RawModel()
{

}