#include "Rendering\RawModelCustom.h"

RawModel::RawModel(std::string fileName)
{
    boost::endian::big_int16_buf_t* test;
    int16_t tal;
    test = &(boost::endian::big_int16_buf_t)tal;

    char* fileData;
    std::ifstream in(fileName.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open())
        LOG_ERROR("Failed to load custom binary model \"%s\"", fileName.c_str());
        
    unsigned int fileByteSize = in.tellg();
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    unsigned int offset = 0;
    ReadMeshFileHeader(offset, fileData, fileByteSize);
    ReadMesh(offset, fileData, fileByteSize);

}

bool RawModel::ReadMeshFileHeader(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    m_Vertices.resize(*fileData);
    offset += sizeof(unsigned int);
    m_Indices.resize(*(fileData + offset));
    offset += sizeof(unsigned int);
#else
#endif
}

bool RawModel::ReadMesh(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
    ReadVertices(offset, fileData, fileByteSize);
    ReadIndices(offset, fileData, fileByteSize);
}

bool RawModel::ReadVertices(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    memcpy(&m_Vertices[0], fileData, m_Vertices.size() * sizeof(Vertex));
    offset += m_Vertices.size() * sizeof(Vertex);
#else
#endif
}

bool RawModel::ReadIndices(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    memcpy(&m_Indices[0], fileData, m_Indices.size() * sizeof(unsigned int));
    offset += m_Indices.size() * sizeof(unsigned int);
#else
#endif
}


RawModel::~RawModel()
{

}