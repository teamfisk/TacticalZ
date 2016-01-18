#ifndef ExplosionEffectJob_h__
#define ExplosionEffectJob_h__

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

struct ExplosionEffectJob : ModelJob
{
    ExplosionEffectJob(ComponentWrapper explosionEffectComponent, ::Model* model, Camera* camera, glm::mat4 matrix, ::RawModel::MaterialGroup matGroup, ComponentWrapper modelComponent, ::World* world)
        : ModelJob(model, camera, matrix, matGroup, modelComponent, world)
    {
        ExplosionOrigin = (glm::vec3)explosionEffectComponent["ExplosionOrigin"];
        TimeSinceDeath = (double)explosionEffectComponent["TimeSinceDeath"];
        ExplosionDuration = (double)explosionEffectComponent["ExplosionDuration"];
        EndColor = (glm::vec4)explosionEffectComponent["EndColor"];
        Randomness = (bool)explosionEffectComponent["Randomness"];
        RandomnessScalar = (double)explosionEffectComponent["RandomnessScalar"];
        Velocity = (glm::vec2)explosionEffectComponent["Velocity"];
        ColorByDistance = (bool)explosionEffectComponent["ColorByDistance"];
        ExponentialAccelaration = (bool)explosionEffectComponent["ExponentialAccelaration"];
    };

    glm::vec3 ExplosionOrigin;
    double TimeSinceDeath = 0.f; //Seconds
    double ExplosionDuration = 2.f;
    //bool Gravity = true;
    //double GravityForce = 1.f; // Speed
    //double ObjectRadius = 2.f; // TODO: Change this for object radius when it's available
    glm::vec4 EndColor;
    bool Randomness = false;
    
    float RandomnessScalar = 1.f;
    glm::vec2 Velocity;
    bool ColorByDistance = false;
    //bool ReverseAnimation = false;
    //bool Wireframe = false;
    bool ExponentialAccelaration = false;

    std::array<float, 20> RandomNumbers = {
        0.3257552917701f,
        0.07601508315467f,
        0.57408909014151f,
        0.0f,
        0.8618231368539f,
        0.074957156588769f,
        0.39413607511396f,
        0.54579346698979f,
        0.83222648353885f,
        0.83635707285086f,
        0.34473986148124f,
        0.98092448710507f,
        0.46346380070944f,
        0.7308761201477f,
        0.70832470371776f,
        0.28268750909841f,
        0.26291620883295f,
        0.07685032816457f,
        0.30760929515008f,
        0.2781575388639f };

    void CalculateHash() override
    {
        Hash = 0;
    }
};

#endif