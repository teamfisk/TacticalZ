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

struct ModelJob : RenderJob
{
    ModelJob(Model* model, Camera* camera, glm::mat4 matrix, ::RawModel::MaterialProperties matProp, ComponentWrapper modelComponent, World* world, glm::vec4 fillColor, float fillPercentage)
        : RenderJob()
    {
        Model = model;
		ModelID = model->ResourceID;
		Type = matProp.type;
		::RawModel::MaterialBasic* matGroup = matProp.material;
		switch(matProp.type){
		case ::RawModel::MaterialType::Basic:
			if (Model->isSkined()) {
				ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram")->ResourceID;
			}
			else {
				//JOHAN TODO: Add Non-skined shader
			}
			TextureID = 0;
			break;
		case ::RawModel::MaterialType::SingleTextures:
			{
				if (Model->isSkined()) {
					ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram")->ResourceID;
				}
				else {
					//JOHAN TODO: Add Non-skined shader
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
				if (Model->isSkined()) {
					ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapProgram")->ResourceID;
				}
				else {
					//JOHAN TODO: Add Non-skinned shader
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
        Entity = modelComponent.EntityID;
        glm::vec3 abspos = Transform::AbsolutePosition(world, modelComponent.EntityID);
        glm::vec3 worldpos = glm::vec3(camera->ViewMatrix() * glm::vec4(abspos, 1));
        Depth = worldpos.z;
        World = world;

        FillColor = fillColor;
        FillPercentage = fillPercentage;
        Skeleton = Model->m_RawModel->m_Skeleton;

        if (world->HasComponent(Entity, "Animation") && Skeleton != nullptr) {
            auto animationComponent = world->GetComponent(Entity, "Animation");
            Animation = model->m_RawModel->m_Skeleton->GetAnimation(animationComponent["Name"]);
            AnimationTime = (double)animationComponent["Time"];
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
    const ::Skeleton::Animation* Animation = nullptr;

    float AnimationTime = 0.f;

    glm::vec4 DiffuseColor;
    glm::vec4 SpecularColor;
    glm::vec4 IncandescenceColor;
    unsigned int StartIndex = 0;
    unsigned int EndIndex = 0;
    World* World;

    glm::vec4 FillColor = glm::vec4(0);
    float FillPercentage = 0.0;

    void CalculateHash() override
    {
        Hash = TextureID + ModelID << 10 + ShaderID << 20;
    }
};

#endif