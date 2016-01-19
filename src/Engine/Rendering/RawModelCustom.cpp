#include "Rendering/RawModelCustom.h"

RawModel::RawModel(std::string& fileName)
{
    ReadMeshFile(fileName);
    ReadMaterialFile(fileName);
}

void RawModel::ReadMeshFile(std::string filePath)
{ 
    char* fileData;
    filePath += ".mesh";
    std::ifstream in(filePath.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
        throw Resource::FailedLoadingException("Open mesh file failed");
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
}

void RawModel::ReadMeshFileHeader(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    m_Vertices.resize(*(unsigned int*)(fileData + offset));
    offset += sizeof(unsigned int);
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

void RawModel::ReadMaterialFile(std::string filePath)
{ 
    char* fileData;
    filePath += ".mtrl";
    std::ifstream in(filePath.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
        throw Resource::FailedLoadingException("Open material file failed");
    }
    unsigned int fileByteSize = in.tellg();
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    unsigned int offset = 0;
    if (fileByteSize > 0) {
        ReadMaterials(offset, fileData, fileByteSize);
    }
    delete fileData;
}

void RawModel::ReadMaterials(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{ 
#ifdef BOOST_LITTLE_ENDIAN
    unsigned int* numMaterials = (unsigned int*)(fileData);
    MaterialGroups.reserve(*numMaterials);
    offset += sizeof(unsigned int);

    for (int i = 0; i < *numMaterials; i++) {
        ReadMaterialSingle(offset, fileData, fileByteSize);
    }
#else
#endif
}

void RawModel::ReadMaterialSingle(unsigned int &offset, char* fileData, unsigned int& fileByteSize)
{ 
    MaterialGroup newMaterial;

#ifdef BOOST_LITTLE_ENDIAN

    if (offset + sizeof(unsigned int) * 4 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Material texture names length failed");
    }

    unsigned int* nameLengths = (unsigned int*)(fileData + offset); 
    offset += sizeof(unsigned int) * 4;

    if (offset + sizeof(float) * 2 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Material specular and reflection values failed");
    }

    newMaterial.SpecularExponent = *(float*)(fileData + offset);
    offset += sizeof(float);
    newMaterial.ReflectionFactor = *(float*)(fileData + offset);
    offset += sizeof(float);

    if (offset + sizeof(unsigned int) * 2 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Material start and end index values failed");
    }

    newMaterial.StartIndex = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);
    newMaterial.EndIndex = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    if (nameLengths[0] > 0) {
        if (offset + nameLengths[0] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material texture path failed");
        }

        newMaterial.TexturePath = (fileData + offset);
        offset += nameLengths[0];
    }

    if (nameLengths[1] > 0) {
        if (offset + nameLengths[1] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material NormalMap path failed");
        }

        newMaterial.NormalMapPath = (fileData + offset);
        offset += nameLengths[1];
    }

    if (nameLengths[2] > 0) {
        if (offset + nameLengths[2] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material SpecularMap path failed");
        }

        newMaterial.SpecularMapPath = (fileData + offset);
        offset += nameLengths[2];
    }

    if (nameLengths[3] > 0) {
        if (offset + nameLengths[3] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material IncandescenceMap path failed");
        }

        newMaterial.IncandescenceMapPath = (fileData + offset);
        offset += nameLengths[3];
    }

#else
#endif

    MaterialGroups.push_back(newMaterial);
}

RawModel::~RawModel()
{

}