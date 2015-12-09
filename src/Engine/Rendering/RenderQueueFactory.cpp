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
    //should really return absolute model matrix based on parents position, scale and orientation
    //GetAbsolutePosition(World* world, ComponentWrapper transformComponent)

    ComponentWrapper transformComponent = world->GetComponent(entity, "Transform");
    glm::vec3 position = transformComponent["Position"];
    glm::vec3 scale = transformComponent["Scale"];
    glm::quat oritentation = transformComponent["Orientation"];

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(oritentation) * glm::scale(scale);
    return modelMatrix;
}

glm::vec3 GetAbsolutePosition(World* world, ComponentWrapper transformComponent)
{
  //  positionComponent.EntityID
    return glm::vec3();
}

void RenderQueueFactory::FillModels(World* world, RenderQueue* renderQueue)
{
    auto models = world->GetComponents("Model");
    if (models == nullptr) {
        return;
    }

    for (auto& modelC : *models) {
        std::string resource = modelC["Resource"];
        if (resource.empty()) {
            continue;
        }
        glm::vec4 color = modelC["Color"];
        Model* model = ResourceManager::Load<Model>(resource);

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

}

