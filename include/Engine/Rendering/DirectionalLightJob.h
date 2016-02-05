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
    DirectionalLightJob(ComponentWrapper transformComponent, ComponentWrapper directionalLightComponent, World* m_World)
        : RenderJob()
    {
        Orientation = Transform::AbsoluteOrientation(m_World, transformComponent.EntityID);
        Direction = glm::vec4(0,0,-1,0) * glm::inverse(Orientation);
        //Direction = glm::vec4((glm::vec3)directionalLightComponent["Direction"], 0.f);
        Color = (glm::vec4)directionalLightComponent["Color"];
        Intensity = (double)directionalLightComponent["Intensity"];
    };

    glm::quat Orientation;
    glm::vec4 Direction;
    glm::vec4 Color;
    float Intensity;

    void CalculateHash() override
    {
        Hash = 0;
    }
};

#endif