#ifndef RenderSystem_h__
#define RenderSystem_h__

#include "../Core/System.h"
#include "RenderQueue.h"
#include "../GLM.h"
#include "../OpenGL.h"
#include "../Core/ResourceManager.h"
#include "ESetCamera.h"
#include "Model.h"

class RenderSystem : public ImpureSystem
{
public:
    RenderSystem(EventBroker* eventBrokerer, RenderQueueCollection* renderQueues);
    
    virtual void Update(World* world, double dt) override;

    static glm::vec3 AbsolutePosition(World* world, EntityID entity);
    static glm::quat AbsoluteOrientation(World* world, EntityID entity);
    static glm::vec3 AbsoluteScale(World* world, EntityID entity);

private:
    RenderQueueCollection* m_RenderQueues;

    void Initialize();
    
    glm::mat4 ModelMatrix(World* world, EntityID entity);

    void FillModels(World* world, RenderQueue* renderQueue);
};

#endif