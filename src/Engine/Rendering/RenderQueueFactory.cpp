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

    while (entity != EntityID_Invalid) {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        EntityID parent = world->GetParent(entity);
        //if (parent != EntityID_Invalid) {
            position += AbsoluteOrientation(world, parent) * (glm::vec3)transform["Position"];
        //} else {
        //    position += (glm::vec3)transform["Position"];
        //}
        entity = parent;
    }
   
    return position;
}

glm::quat RenderQueueFactory::AbsoluteOrientation(World* world, EntityID entity)
{
    glm::quat orientation;

    while (entity != EntityID_Invalid) {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        orientation = glm::quat((glm::vec3)transform["Orientation"]) * orientation;
        entity = world->GetParent(entity);
    }
    
    return orientation;
}

glm::vec3 RenderQueueFactory::AbsoluteScale(World* world, EntityID entity)
{
    glm::vec3 scale(1.f);

    while (entity != EntityID_Invalid) {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        scale *= (glm::vec3)transform["Scale"];
        entity = world->GetParent(entity);
    }

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
    }
}

void RenderQueueFactory::FillLights(World* world, RenderQueue* renderQueue)
{
    auto pointLights = world->GetComponents("PointLight");
    if(pointLights == nullptr) {
        return;
    }

    for(auto& pointlightC : *pointLights) {
        bool visible = pointlightC["Visible"];
        if(!visible) {
            continue;
        }
        auto transformC = world->GetComponent(pointlightC.EntityID, "Transform");
        if(&transformC == nullptr) {
            return;
        }

        glm::vec4 color = pointlightC["Color"];
        float radius = (double)pointlightC["Radius"];
        float intensity = (double)pointlightC["Intensity"];
        float falloff = (double)pointlightC["Falloff"];

        PointLightJob job;
        job.Position = glm::vec4(AbsolutePosition(world, transformC.EntityID), 1.f);
        job.Color = color;
        job.Radius = radius;
        job.Intensity = intensity;
        job.Falloff = falloff;

        renderQueue->Add(job);
    }
}

