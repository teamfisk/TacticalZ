#ifndef DirectionalLightJob_h__
#define DirectionalLightJob_h__

#include <cstdint>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/ComponentWrapper.h"
#include "RenderJob.h"
#include "../Core/Transform.h"
#include "../Core/World.h"

struct DirectionalLightJob : RenderJob
{
    DirectionalLightJob(ComponentWrapper transformComponent, ComponentWrapper directionalLightCompoentn, World* m_World)
        : RenderJob()
    {
        Direction = glm::vec4((glm::vec3)directionalLightCompoentn["Direction"], 1.f);
        Color = (glm::vec4)directionalLightCompoentn["Color"];
        Intensity = (double)directionalLightCompoentn["Intensity"];
    };

    glm::vec4 Direction;
    glm::vec4 Color;
    float Intensity;
    glm::vec3 padding = glm::vec3(1.f, 2.f, 3.f);

    void CalculateHash() override
    {
        Hash = 0;
    }
};

#endif