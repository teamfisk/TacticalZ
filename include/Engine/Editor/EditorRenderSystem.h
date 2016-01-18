#ifndef EditorRenderSystem_h__
#define EditorRenderSystem_h__

#include "../Core/System.h"
#include "../Rendering/IRenderer.h"
#include "../Rendering/ModelJob.h"
#include "../Rendering/Camera.h"
#include "../Rendering/ESetCamera.h"

class EditorRenderSystem : public ImpureSystem
{
public:
    EditorRenderSystem(World* world, EventBroker* eventBroker, IRenderer* renderer, RenderFrame* renderFrame);

    virtual void Update(double dt) override;

private:
    IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    Camera* m_EditorCamera;
    EntityWrapper m_CurrentCamera = EntityWrapper::Invalid;

    EventRelay<EditorRenderSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(Events::SetCamera& e);
};

#endif