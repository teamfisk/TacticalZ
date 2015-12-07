#include "Rendering/RenderQueueFactory.h"


RenderQueueFactory::RenderQueueFactory()
{
    m_RenderQueues = RenderQueueCollection();
}

void RenderQueueFactory::Update(World* world)
{
    m_RenderQueues.Clear();
    FillModels(world, &m_RenderQueues.Forward);
    FillSprites(world, &m_RenderQueues.Sprites);
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
   for(auto& modelC : world->GetComponents("Model")) {
       ModelJob job;
       std::string modelFile = modelC["ModelFile"];
       glm::vec4 color = modelC["Color"];
       Model* model = ResourceManager::Load<Model>(modelFile);

       for (auto texGroup : model->TextureGroups) {
           job.TextureID = (texGroup.Texture) ? texGroup.Texture->ResourceID : 0;
           job.DiffuseTexture = texGroup.Texture.get();
           job.NormalTexture = texGroup.NormalMap.get();
           job.SpecularTexture = texGroup.SpecularMap.get();
           job.Model = model;
           job.StartIndex = texGroup.StartIndex;
           job.EndIndex = texGroup.EndIndex;
           job.ModelMatrix = model->m_Matrix * ModelMatrix(world, modelC.EntityID);
           job.Color = color;

           renderQueue->Add(job);
       }
   }
}

void RenderQueueFactory::FillSprites(World* world, RenderQueue* renderQueue)
{
    
}

void RenderQueueFactory::FillLights(World* world, RenderQueue* renderQueue)
{

}

