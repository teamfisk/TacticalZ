#ifndef PointLightJob_h__
#define PointLightJob_h__

#include <cstdint>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/ComponentWrapper.h"
#include "RenderJob.h"

struct PointLightJob : RenderJob
{
    PointLightJob(ComponentWrapper transformComponent, ComponentWrapper pointLightComponent)
        : RenderJob()
    {
        Position = glm::vec4((glm::vec3)transformComponent["Position"], 1.0f);
        Color = (glm::vec4)pointLightComponent["Color"];
        Radius = (double)pointLightComponent["Radius"];
        Intensity = (double)pointLightComponent["Intensity"];
        Falloff = (double)pointLightComponent["Falloff"];
    };

    glm::vec4 Position;
    glm::vec4 Color;
    float Radius;
    float Intensity;
    float Falloff;
    float padding = 123;

    void CalculateHash() override
    {
        Hash = 0;
    }
};

#endif