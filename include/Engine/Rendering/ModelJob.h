#ifndef ModelJob_h__
#define ModelJob_h__

#include <cstdint>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/ComponentWrapper.h"
#include "Texture.h"
#include "Model.h"
#include "RenderJob.h"
#include "../Core/ResourceManager.h"
#include "Camera.h"
#include "../Core/World.h"
#include "../Core/Transform.h"
#include "Skeleton.h"
#include "ShaderProgram.h"
#include "BlendTree.h"

struct ModelJob : RenderJob
{
    ModelJob(Model* model, Camera* camera, glm::mat4 matrix, ::RawModel::MaterialProperties matProp, ComponentWrapper modelComponent, World* world, glm::vec4 fillColor, float fillPercentage, bool isShielded)
        : RenderJob()
    {
        Model = model;
		ModelID = model->ResourceID;
		Type = matProp.type;
		::RawModel::MaterialBasic* matGroup = matProp.material;
		switch(matProp.type){
		case ::RawModel::MaterialType::Basic:
			if (Model->IsSkinned()) {
				ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSkinnedProgram")->ResourceID;
			}
			else {
				ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram")->ResourceID;
			}
			TextureID = 0;
			break;
		case ::RawModel::MaterialType::SingleTextures:
			{
				if (Model->IsSkinned()) {
					ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSkinnedProgram")->ResourceID;
				}
				else {
					ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram")->ResourceID;
				}
				::RawModel::MaterialSingleTextures* singleTextures = static_cast<::RawModel::MaterialSingleTextures*>(matProp.material);
				TextureID = (singleTextures->ColorMap.Texture) ? singleTextures->ColorMap.Texture->ResourceID : 0;
				if (modelComponent["DiffuseTexture"]) {
					DiffuseTexture.push_back(&singleTextures->ColorMap);
				}

				if (modelComponent["NormalMap"]) {
					NormalTexture.push_back(&singleTextures->NormalMap);
				}

				if (modelComponent["SpecularMap"]) {
					SpecularTexture.push_back(&singleTextures->SpecularMap);
				}

				if (modelComponent["GlowMap"]) {
					IncandescenceTexture.push_back(&singleTextures->IncandescenceMap);
				}
			}
			break;
		case ::RawModel::MaterialType::SplatMapping:
			{
				if (Model->IsSkinned()) {
					ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapSkinnedProgram")->ResourceID;
				}
				else {
					ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapProgram")->ResourceID;
				}
				::RawModel::MaterialSplatMapping* SplatTextures = static_cast<::RawModel::MaterialSplatMapping*>(matProp.material);

				SplatMap = &SplatTextures->SplatMap;

				TextureID = (SplatTextures->ColorMaps[0].Texture) ? SplatTextures->ColorMaps[0].Texture->ResourceID : 0;
				if (modelComponent["DiffuseTexture"]) {
					for (auto& texture : SplatTextures->ColorMaps) {
						DiffuseTexture.push_back(&texture);
					}
				}

				if (modelComponent["NormalMap"]) {
					for (auto& texture : SplatTextures->NormalMaps) {
						NormalTexture.push_back(&texture);
					}
				}

				if (modelComponent["SpecularMap"]) {
					for (auto& texture : SplatTextures->SpecularMaps) {
						SpecularTexture.push_back(&texture);
					}
				}

				if (modelComponent["GlowMap"]) {
					for (auto& texture : SplatTextures->IncandescenceMaps) {
						IncandescenceTexture.push_back(&texture);
					}
				}
			}
			break;
		}
        DiffuseColor = matGroup->DiffuseColor;
        SpecularColor = matGroup->SpecularColor;
        IncandescenceColor = matGroup->IncandescenceColor;
        StartIndex = matGroup->StartIndex;
        EndIndex = matGroup->EndIndex;
        Matrix = matrix;
        Color = modelComponent["Color"];
        GlowIntensity = ((double)modelComponent["GlowIntensity"]);
        Entity = modelComponent.EntityID;
        glm::vec3 abspos = glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
        glm::vec3 worldpos = glm::vec3(camera->ViewMatrix() * glm::vec4(abspos, 1));
        Depth = worldpos.z;
        World = world;

        FillColor = fillColor;
        FillPercentage = fillPercentage;
		IsShielded = isShielded;

        if (model->IsSkinned()) {
            Skeleton = Model->m_RawModel->m_Skeleton;

            if (Skeleton != nullptr) {
                
                EntityWrapper entityWrapper = EntityWrapper(world, modelComponent.EntityID);

                if(Skeleton->BlendTrees.find(entityWrapper) != Skeleton->BlendTrees.end()) {
                    BlendTree = Skeleton->BlendTrees.at(entityWrapper);
                }
            }
        }
    };

    unsigned int TextureID;
    unsigned int ShaderID;
	unsigned int ModelID;

	::RawModel::MaterialType Type;
    EntityID Entity;
    glm::mat4 Matrix;
	const ::RawModel::TextureProperties* SplatMap;
    std::vector<const ::RawModel::TextureProperties*> DiffuseTexture;
	std::vector<const ::RawModel::TextureProperties*> NormalTexture;
	std::vector<const ::RawModel::TextureProperties*> SpecularTexture;
	std::vector<const ::RawModel::TextureProperties*> IncandescenceTexture;
    float Shininess = 0.f;
    glm::vec4 Color;
    const ::Model* Model = nullptr;
    ::Skeleton* Skeleton = nullptr;
    std::shared_ptr<::BlendTree> BlendTree = nullptr;
    float GlowIntensity = 8.0;
    glm::vec4 DiffuseColor;
    glm::vec4 SpecularColor;
    glm::vec4 IncandescenceColor;
    unsigned int StartIndex = 0;
    unsigned int EndIndex = 0;
    World* World;

    glm::vec4 FillColor = glm::vec4(0);
    float FillPercentage = 0.0;
	bool IsShielded;
    void CalculateHash() override
    {
		Hash = TextureID;
		Hash += ModelID << 10;
		Hash += ShaderID << 20;
    }
};

#endif