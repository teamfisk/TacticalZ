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
#include "Camera.h"
#include "ModelJob.h"
#include "Renderer.h"
#include "PointLightJob.h"
#include "../Core/Transform.h"
#include "DebugCameraInputController.h"

class RenderSystem : public ImpureSystem
{
public:
    RenderSystem(EventBroker* eventBrokerer, const IRenderer* renderer, RenderFrame* renderFrame);
    ~RenderSystem();

    virtual void Update(World* world, double dt) override;

private:
    World* m_World = nullptr;
    const IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    Camera* m_Camera;
    EntityWrapper m_CurrentCamera = EntityWrapper::Invalid;

    EventRelay<RenderSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(Events::SetCamera &event);
    EventRelay<RenderSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);

    void fillModels(std::list<std::shared_ptr<RenderJob>>& jobs, World* world);
    void fillLight(std::list<std::shared_ptr<RenderJob>>& jobs, World* world);
};

#endif