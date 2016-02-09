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

struct ModelJob : RenderJob
{
    ModelJob(Model* model, Camera* camera, glm::mat4 matrix, ::RawModel::MaterialGroup matGroup, ComponentWrapper modelComponent, World* world, glm::vec4 fillColor, float fillPercentage)
        : RenderJob()
    {
        Model = model;
        TextureID = (matGroup.Texture) ? matGroup.Texture->ResourceID : 0;
        if (modelComponent["DiffuseTexture"]) {
            DiffuseTexture = matGroup.Texture.get();
        } else {
            DiffuseTexture = nullptr;
        }
        if (modelComponent["NormalMap"]) {
            NormalTexture = matGroup.NormalMap.get();
        } else {
            NormalTexture = nullptr;
        }
        if (modelComponent["SpecularMap"]) {
            SpecularTexture = matGroup.SpecularMap.get();
        } else {
            SpecularTexture = nullptr;
        }
        if (modelComponent["GlowMap"]) {
            IncandescenceTexture = matGroup.IncandescenceMap.get();
        } else {
            IncandescenceTexture = nullptr;
        }
        DiffuseColor = matGroup.DiffuseColor;
        SpecularColor = matGroup.SpecularColor;
        IncandescenceColor = matGroup.IncandescenceColor;
        StartIndex = matGroup.StartIndex;
        EndIndex = matGroup.EndIndex;
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

        if (Skeleton != nullptr) {
            if (world->HasComponent(Entity, "Animation")) {
                auto animationComponent = world->GetComponent(Entity, "Animation");

                for (int i = 1; i <= 3; i++) {
                    ::Skeleton::AnimationData animationData;
                    animationData.animation = model->m_RawModel->m_Skeleton->GetAnimation(animationComponent["AnimationName" + std::to_string(i)]);
                    if (animationData.animation == nullptr) {
                        continue;
                    }
                    animationData.time = (double)animationComponent["Time" + std::to_string(i)];
                    animationData.weight = (double)animationComponent["Weight" + std::to_string(i)];

                    Animations.push_back(animationData);
                }
            }

            if (world->HasComponent(Entity, "AnimationOffset")) {
                auto animationOffsetComponent = world->GetComponent(Entity, "AnimationOffset");
                AnimationOffset.animation = model->m_RawModel->m_Skeleton->GetAnimation(animationOffsetComponent["AnimationName"]);
                AnimationOffset.time = (double)animationOffsetComponent["Time"];
            } else {
                AnimationOffset.animation = nullptr;
            }
        }
    };

    unsigned int TextureID;
    unsigned int ShaderID;

    EntityID Entity;
    glm::mat4 Matrix;
    const Texture* DiffuseTexture;
    const Texture* NormalTexture;
    const Texture* SpecularTexture;
    const Texture* IncandescenceTexture;
    float Shininess = 0.f;
    glm::vec4 Color;
    const ::Model* Model = nullptr;
    ::Skeleton* Skeleton = nullptr;
   // const ::Skeleton::Animation* Animation = nullptr;

    std::vector<::Skeleton::AnimationData> Animations;
    ::Skeleton::AnimationOffset AnimationOffset;

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
        Hash = TextureID;
    }
};

#endif