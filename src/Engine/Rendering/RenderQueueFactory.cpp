#include "Rendering/RenderQueueFactory.h"


RenderQueueFactory::RenderQueueFactory()
{
    m_RenderQueues = RenderQueueCollection();
}

void RenderQueueFactory::Update(World* world)
{
    m_RenderQueues.Clear();
    FillModels(world, &m_RenderQueues.Forward);
    FillLights(world, &m_RenderQueues.Lights);
}

glm::mat4 RenderQueueFactory::ModelMatrix(World* world, EntityID entity)
{
    glm::vec3 position = AbsolutePosition(world, entity);
    glm::quat orientation = AbsoluteOrientation(world, entity);
    glm::vec3 scale = AbsoluteScale(world, entity);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation) * glm::scale(scale);
    return modelMatrix;
}

glm::vec3 RenderQueueFactory::AbsolutePosition(World* world, EntityID entity)
{
    glm::vec3 position;

    do {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        EntityID parent = world->GetParent(entity);
        if (parent != 0) {
            position += AbsoluteOrientation(world, parent) * (glm::vec3)transform["Position"];
        } else {
            position += (glm::vec3)transform["Position"];
        }
        entity = parent;
    } while (entity != 0);
   
    return position;
}

glm::quat RenderQueueFactory::AbsoluteOrientation(World* world, EntityID entity)
{
    glm::quat orientation;

    do {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        orientation = glm::quat((glm::vec3)transform["Orientation"]) * orientation;
        entity = world->GetParent(entity);
    } while (entity != 0);
    
    return orientation;
}

glm::vec3 RenderQueueFactory::AbsoluteScale(World* world, EntityID entity)
{
    glm::vec3 scale(1.f);

    do {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        scale *= (glm::vec3)transform["Scale"];
        entity = world->GetParent(entity);
    } while (entity != 0);

    return scale;
}

void RenderQueueFactory::FillModels(World* world, RenderQueue* renderQueue)
{
    auto models = world->GetComponents("Model");
    if (models == nullptr) {
        return;
    }

    for (auto& modelC : *models) {
        bool visible = modelC["Visible"];
        if (!visible) {
            continue;
        }
        std::string resource = modelC["Resource"];
        if (resource.empty()) {
            continue;
        }
        glm::vec4 color = modelC["Color"];
        Model* model = ResourceManager::Load<Model>(resource);
        if (model == nullptr) {
            model = ResourceManager::Load<Model>("Models/Core/Error.obj");
        }

        for (auto texGroup : model->TextureGroups) {
            bool hasDeathComp = world->HasComponent(modelC.EntityID, "CoolDeathAnim");
            if (!hasDeathComp) {
                ModelJob job;

                job.TextureID = (texGroup.Texture) ? texGroup.Texture->ResourceID : 0;
                job.DiffuseTexture = texGroup.Texture.get();
                job.NormalTexture = texGroup.NormalMap.get();
                job.SpecularTexture = texGroup.SpecularMap.get();
                job.Model = model;
                job.StartIndex = texGroup.StartIndex;
                job.EndIndex = texGroup.EndIndex;
                job.ModelMatrix = model->m_Matrix * ModelMatrix(world, modelC.EntityID);
                job.Color = color;

                //TODO: RENDERER: Not sure if the best solution for pickingColor to entity link is this
                job.Entity = modelC.EntityID;

                renderQueue->Add(job);
            }
            else {
                CoolDeathAnimationJob job;

                auto deathComp = world->GetComponent(modelC.EntityID, "CoolDeathAnim");

                job.ExplosionOrigin = (glm::vec3)deathComp["ExplosionOrigin"];
                job.TimeSinceDeath = (double)deathComp["TimeSinceDeath"];
                job.ExplosionDuration = (double)deathComp["ExplosionDuration"];
                job.Gravity = (bool)deathComp["Gravity"];
                job.GravityForce = (double)deathComp["GravityForce"];
                job.ObjectRadius = (double)deathComp["ObjectRadius"];
                job.EndColor = (glm::vec4)deathComp["EndColor"];
                job.Randomness = (bool)deathComp["Randomness"];
                job.RandomnessScalar = (double)deathComp["RandomnessScalar"];
                job.Acceleration = (glm::vec2)deathComp["Acceleration"];
                job.ColorPerPolygon = (bool)deathComp["ColorPerPolygon"];
                job.ReverseAnimation = (bool)deathComp["ReverseAnimation"];
                job.Wireframe = (bool)deathComp["Wireframe"];
                job.RandomNumbers = {
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

                job.TextureID = (texGroup.Texture) ? texGroup.Texture->ResourceID : 0;
                job.DiffuseTexture = texGroup.Texture.get();
                job.NormalTexture = texGroup.NormalMap.get();
                job.SpecularTexture = texGroup.SpecularMap.get();
                job.Model = model;
                job.StartIndex = texGroup.StartIndex;
                job.EndIndex = texGroup.EndIndex;
                job.ModelMatrix = model->m_Matrix * ModelMatrix(world, modelC.EntityID);
                job.Color = color;

                //TODO: RENDERER: Not sure if the best solution for pickingColor to entity link is this
                job.Entity = modelC.EntityID;

                renderQueue->Add(job);
            }
        }
    }
}

void RenderQueueFactory::FillLights(World* world, RenderQueue* renderQueue)
{

}

