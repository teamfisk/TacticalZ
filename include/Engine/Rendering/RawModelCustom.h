#ifndef RawModelCustom_h__
#define RawModelCustom_h__

#ifndef USING_ASSIMP_AS_IMPORTER

#define RawModel RawModelCustom

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



class RawModelCustom : public Resource
{
    friend class ResourceManager;

protected:
    RawModelCustom(std::string fileName);

public:
    ~RawModelCustom();
    
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
        float SpecularExponent;
        float ReflectionFactor;
        float DiffuseColor[3];
        float SpecularColor[3];
        float IncandescenceColor[3];
        unsigned int StartIndex;
        unsigned int EndIndex;
        //float Transparency;
        std::string TexturePath;
        std::shared_ptr<::Texture> Texture;
        std::string NormalMapPath;
        std::shared_ptr<::Texture> NormalMap;
        std::string SpecularMapPath;
        std::shared_ptr<::Texture> SpecularMap;
        std::string IncandescenceMapPath;
        std::shared_ptr<::Texture> IncandescenceMap;
    };

    std::vector<MaterialGroup> MaterialGroups;

    std::vector<Vertex> m_Vertices;
    std::vector<unsigned int> m_Indices;
    Skeleton* m_Skeleton = nullptr;
    glm::mat4 m_Matrix;

private:
   

    void ReadMeshFile(std::string filePath);
    void ReadMeshFileHeader(unsigned int& offset, char* fileData, unsigned int& fileByteSize);
    void ReadMesh(unsigned int& offset, char* fileData, unsigned int& fileByteSize);
    void ReadVertices(unsigned int& offset, char* fileData, unsigned int& fileByteSize);
    void ReadIndices(unsigned int& offset, char* fileData, unsigned int& fileByteSize);

    void ReadMaterialFile(std::string filePath);
    void ReadMaterials(unsigned int &offset, char* fileData, unsigned int& fileByteSize);
    void ReadMaterialSingle(unsigned int &offset, char* fileData, unsigned int& fileByteSize);

    void ReadAnimationFile(std::string filePath);
    void ReadAnimationBindPoses(unsigned int &offset, char* fileData, unsigned int& fileByteSize);
    void ReadAnimationJoint(unsigned int &offset, char* fileData, unsigned int& fileByteSize);
    void ReadAnimationClips(unsigned int &offset, char* fileData, unsigned int& fileByteSize, unsigned int numberOfClips);
    void ReadAnimationClipSingle(unsigned int &offset, char* fileData, unsigned int& fileByteSize, unsigned int clipIndex);
    void ReadAnimationKeyFrame(unsigned int &offset, char* fileData, unsigned int& fileByteSize, unsigned int numberOfJoints, Skeleton::Animation& animation);
    
    //void CreateSkeleton(std::vector<std::tuple<std::string, glm::mat4>> &boneInfo, std::map<std::string, int> &boneNameMapping, aiNode* node, int parentID);
};

#else

#include "RawModelAssimp.h"

#endif
#endif