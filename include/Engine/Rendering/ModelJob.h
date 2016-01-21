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

struct ModelJob : RenderJob
{
    ModelJob(Model* model, Camera* camera, glm::mat4 matrix, ::RawModel::MaterialGroup matGroup, ComponentWrapper modelComponent, World* world)
        : RenderJob()
    {
        Model = model;
        TextureID = (matGroup.Texture) ? matGroup.Texture->ResourceID : 0;
        DiffuseTexture = matGroup.Texture.get();
        NormalTexture = matGroup.NormalMap.get();
        SpecularTexture = matGroup.SpecularMap.get();
        StartIndex = matGroup.StartIndex;
        EndIndex = matGroup.EndIndex;
        Matrix = matrix;
        Color = modelComponent["Color"];
        Entity = modelComponent.EntityID;
        glm::vec3 abspos = Transform::AbsolutePosition(world, modelComponent.EntityID);
        glm::vec3 worldpos = glm::vec3(camera->ViewMatrix() * glm::vec4(abspos, 1));
        Depth = worldpos.z;
        World = world;
    };

    unsigned int TextureID;
    unsigned int ShaderID;

    EntityID Entity;
    glm::mat4 Matrix;
    const Texture* DiffuseTexture;
    const Texture* NormalTexture;
    const Texture* SpecularTexture;
    float Shininess = 0.f;
    glm::vec4 Color;
    const ::Model* Model = nullptr;
    unsigned int StartIndex = 0;
    unsigned int EndIndex = 0;
    World* World;

    void CalculateHash() override
    {
        Hash = TextureID;
    }
};

#endif