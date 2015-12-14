#ifndef RenderQueueFactory_h__
#define RenderQueueFactory_h__

#include "../Core/World.h"
#include "RenderQueue.h"
#include "../Core/ResourceManager.h"
#include "Model.h"
#include "../GLM.h"
#include "../Core/EventBroker.h"
#include "ESetCamera.h"

class RenderQueueFactory
{
public:
    RenderQueueFactory(EventBroker* eventBroker);
    void Update(World* world);

    
    RenderQueueCollection RenderQueues() const { return m_RenderQueues; }
private:
    EventBroker* m_EventBroker;
    RenderQueueCollection m_RenderQueues;

    void FillModels(World* world, RenderQueue* renderQueue);
    void FillLights(World* world, RenderQueue* renderQueue);

    glm::mat4 ModelMatrix(World* world, EntityID entity);

    glm::vec3 AbsolutePosition(World* world, EntityID entity);
    glm::quat AbsoluteOrientation(World* world, EntityID entity);
    glm::vec3 AbsoluteScale(World* world, EntityID entity);

    EntityID m_CurrentCamera;

};

#endif