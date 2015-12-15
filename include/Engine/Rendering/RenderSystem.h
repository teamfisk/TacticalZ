#ifndef RenderSystem_h__
#define RenderSystem_h__

#include "../Core/System.h"
#include "RenderQueue.h"
#include "../GLM.h"
#include "../OpenGL.h"
#include "../Core/ResourceManager.h"
#include "ESetCamera.h"
#include "Model.h"
#include "../Core/EKeyDown.h"
#include "../Input/EInputCommand.h"

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
    bool m_SwitchCamera = false;

    EventRelay<RenderSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(const Events::SetCamera &event);
    EntityID m_CurrentCamera = -1;

    void SwitchCamera(EntityID entity);

    void Initialize();
    void UpdateViewMatrix(ComponentWrapper& cameraTransform);
    void UpdateProjectionMatrix(ComponentWrapper& cameraComponent);
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;
    
    glm::mat4 ModelMatrix(World* world, EntityID entity);
    void FillModels(World* world, RenderQueue* renderQueue);

    EventRelay<RenderSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
};

#endif