#ifndef RawModelCustom_h__
#define RawModelCustom_h__

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <stack>

#include <boost/filesystem/path.hpp>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/ResourceManager.h"
#include "Texture.h"
#include "Skeleton.h"

#include "boost\endian\buffers.hpp"

class RawModel : public Resource
{
    friend class ResourceManager;

protected:
    RawModel(std::string fileName);

public:
    ~RawModel();
    
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec3 BiNormal;
        glm::vec2 TextureCoords;
        glm::vec4 BoneIndices;
        glm::vec4 BoneWeights;
    };

    struct MaterialGroup
    {
        float Shininess;
        std::shared_ptr<::Texture> Texture;
        std::shared_ptr<::Texture> NormalMap;
        std::shared_ptr<::Texture> SpecularMap;
        unsigned int StartIndex;
        unsigned int EndIndex;
    };

    std::vector<MaterialGroup> TextureGroups;

    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;
    Skeleton* m_Skeleton = nullptr;
    glm::mat4 m_Matrix;

private:

    bool ReadMeshFileHeader(unsigned int& offset, char* fileData, unsigned int& fileByteSize);
    bool ReadMesh(unsigned int& offset, char* fileData, unsigned int& fileByteSize);
    bool ReadVertices(unsigned int& offset, char* fileData, unsigned int& fileByteSize);
    bool ReadIndices(unsigned int& offset, char* fileData, unsigned int& fileByteSize);

    //void CreateSkeleton(std::vector<std::tuple<std::string, glm::mat4>> &boneInfo, std::map<std::string, int> &boneNameMapping, aiNode* node, int parentID);
};

#endif
