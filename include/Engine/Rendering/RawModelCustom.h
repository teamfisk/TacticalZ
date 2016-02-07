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
	};

    struct SkinedVertex : public Vertex {
        glm::vec4 BoneIndices;
        glm::vec4 BoneWeights;
    };

	struct TextureProperties {
		std::string TexturePath;
		glm::vec2 UVRepeat;
		std::shared_ptr<::Texture> Texture;	
	};
	
    struct MaterialBasic
    {
        float SpecularExponent;
        float ReflectionFactor;
        glm::vec4 DiffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 SpecularColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        glm::vec4 IncandescenceColor{ 1.0f, 1.0f, 1.0f, 1.0f };
        unsigned int StartIndex;
        unsigned int EndIndex;
        //float Transparency;
    };

	struct MaterialSplatMapping : public MaterialBasic
	{
		TextureProperties SplatMap;
		std::vector<TextureProperties> ColorMaps;
		std::vector<TextureProperties> NormalMaps;
		std::vector<TextureProperties> SpecularMaps;
		std::vector<TextureProperties> IncandescenceMaps;
	};

	struct MaterialSingleTextures : public MaterialBasic
	{
		TextureProperties ColorMap;
		TextureProperties NormalMap;
		TextureProperties SpecularMap;
		TextureProperties IncandescenceMap;
	};

	enum class MaterialType { Basic = 1, SplatMapping, SingleTextures };

	struct MaterialProperties {
		MaterialType type;
		MaterialBasic* material;
	};

	const Vertex*	Vertices() const {
		if (hasSkin) { 
			return m_SkinedVertices.data(); 
		} else {
			return m_Vertices.data();
		}
	};

	unsigned int	VertexSize() const {
		if (hasSkin) {
			return sizeof(SkinedVertex);
		}
		else {
			return sizeof(Vertex);
		}
	};

	unsigned int	NumVertices() const {
		if (hasSkin) {
			return m_SkinedVertices.size();
		} else {
			return m_Vertices.size();
		}
	};

	bool isSkined() const { return hasSkin; };

	std::vector<MaterialProperties> m_Materials;

    std::vector<unsigned int> m_Indices;
    Skeleton* m_Skeleton = nullptr;
    glm::mat4 m_Matrix;

private:
   	bool hasSkin;
	std::vector<Vertex> m_Vertices;
	std::vector<SkinedVertex> m_SkinedVertices;

    void ReadMeshFile(std::string filePath);
    void ReadMeshFileHeader(std::size_t& offset, char* fileData);
    void ReadMesh(std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
    void ReadVertices(std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
    void ReadIndices(std::size_t& offset, char* fileData, const unsigned int& fileByteSize);

    void ReadMaterialFile(std::string filePath);
    void ReadMaterials(std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
    void ReadMaterialSingle(std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
	void ReadMaterialBasic(MaterialBasic* Material, std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
	void ReadMaterialSingleTexture(MaterialSingleTextures* Material, std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
	void ReadMaterialSplatMapping(MaterialSplatMapping* Material, std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
	void ReadMaterialTextureProperties(TextureProperties& texture, std::size_t& offset, char* fileData, const unsigned int& fileByteSize);

    void ReadAnimationFile(std::string filePath);
    void ReadAnimationBindPoses(std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
    void ReadAnimationJoint(std::size_t& offset, char* fileData, const unsigned int& fileByteSize);
    void ReadAnimationClips(std::size_t& offset, char* fileData, const unsigned int& fileByteSize, unsigned int numberOfClips);
    void ReadAnimationClipSingle(std::size_t& offset, char* fileData, const unsigned int& fileByteSize, unsigned int clipIndex);
    void ReadAnimationKeyFrame(std::size_t& offset, char* fileData, const unsigned int& fileByteSize, std::vector<Skeleton::Animation::Keyframe>& animation);
    
    //void CreateSkeleton(std::vector<std::tuple<std::string, glm::mat4>> &boneInfo, std::map<std::string, int> &boneNameMapping, aiNode* node, int parentID);
};

#else

#include "RawModelAssimp.h"

#endif
#endif