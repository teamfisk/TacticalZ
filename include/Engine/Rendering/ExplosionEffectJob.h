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
    ExplosionEffectJob(ComponentWrapper explosionEffectComponent, ::Model* model, Camera* camera, glm::mat4 matrix, ::RawModel::MaterialProperties matGroup, ComponentWrapper modelComponent, ::World* world, glm::vec4 fillColor, float fillPercentage)
        : ModelJob(model, camera, matrix, matGroup, modelComponent, world, fillColor, fillPercentage)
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
		Reverse = (bool)explosionEffectComponent["Reverse"];
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
	bool Reverse = false;

    std::array<float, 50> RandomNumbers = {
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
        0.2781575388639f   ,
        0.016950582161988f  ,
        0.25787915254844f   ,
        0.76293767279151f   ,
        0.67730279903733f   ,
        0.49042877018937f   ,
        0.13002237823327f   ,
        0.62803373841012f   ,
        0.94525353747665f   ,
        0.32893980076953f   ,
        0.95952951719962f   ,
        0.056386206790985f  ,
        0.012030893476694f  ,
        0.93090049220757f,
        0.83579883064879f,
        0.79733513938139f,
        0.8796381046435f    ,
        0.94160434600972f   ,
        0.76901855588379f   ,
        0.50253688101775f   ,
        0.82457069578793f   ,
        0.80157403359263f   ,
        0.87291810189975f   ,
        0.64679548919517f   ,
        0.42664138899494f   ,
        0.77171308303797f   ,
        0.75567959237643f   ,
        0.45190826265696f   ,
        0.59844922628181f   ,
        0.56534937655802f   ,
        0.195656257307f};

    void CalculateHash() override
    {
        Hash = 0;
    }
};

#endif