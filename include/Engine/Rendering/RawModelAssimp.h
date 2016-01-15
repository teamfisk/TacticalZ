#ifndef RawModelAssimp_h__
#define RawModelAssimp_h__

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <stack>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <boost/filesystem/path.hpp>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/ResourceManager.h"
#include "Texture.h"
#include "Skeleton.h"

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
        float Transparency;
        std::string TexturePath;
		std::shared_ptr<::Texture> Texture;
        std::string NormalMapPath;
		std::shared_ptr<::Texture> NormalMap;
        std::string SpecularMapPath;
		std::shared_ptr<::Texture> SpecularMap;
		unsigned int StartIndex;
		unsigned int EndIndex;
	};

	std::vector<MaterialGroup> MaterialGroups;

	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
	Skeleton* m_Skeleton = nullptr;
	glm::mat4 m_Matrix;

private:
	std::vector<glm::ivec2> BoneIndices;
	std::vector<glm::vec2> BoneWeights;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec3> TangentNormals;
	std::vector<glm::vec3> BiTangentNormals;
	std::vector<glm::vec2> TextureCoords;

	void CreateSkeleton(std::vector<std::tuple<std::string, glm::mat4>> &boneInfo, std::map<std::string, int> &boneNameMapping, aiNode* node, int parentID);
};

#endif
