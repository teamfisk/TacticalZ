#ifndef SpriteJob_h__
#define SpriteJob_h__

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

struct SpriteJob : RenderJob
{
    SpriteJob(ComponentWrapper cSprite, Camera* camera, glm::mat4 matrix, World* world, glm::vec4 fillColor, float fillPercentage)
        : RenderJob()
    {
        Model = ResourceManager::Load<::Model>("Models/Core/UnitQuad.mesh");
        ::RawModel::MaterialGroup matGroup = Model->MaterialGroups().front();
        TextureID = (matGroup.Texture) ? matGroup.Texture->ResourceID : 0;

        DiffuseTexture = CommonFunctions::LoadTexture(cSprite["DiffuseTexture"], true);

        IncandescenceTexture = CommonFunctions::LoadTexture(cSprite["GlowMap"], true);

        StartIndex = matGroup.StartIndex;
        EndIndex = matGroup.EndIndex;
        Matrix = matrix;
        Color = cSprite["Color"];
        Entity = cSprite.EntityID;
        Position = Transform::AbsolutePosition(world, cSprite.EntityID);
        glm::vec3 viewpos = glm::vec3(camera->ViewMatrix() * glm::vec4(Position, 1));
        Depth = viewpos.z;
        World = world;

        FillColor = fillColor;
        FillPercentage = fillPercentage;
    };

    unsigned int TextureID;

    EntityID Entity;
    glm::mat4 Matrix;
    const Texture* DiffuseTexture;
    const Texture* NormalTexture;
    const Texture* SpecularTexture;
    const Texture* IncandescenceTexture;
    float Shininess = 0.f;
    glm::vec4 Color;
    glm::vec3 Position;
    const ::Model* Model = nullptr;
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