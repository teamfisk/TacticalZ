#ifndef EditorWidgetRenderSystem_h__
#define EditorWidgetRenderSystem_h__

#include "../Core/System.h"
#include "../Rendering/IRenderer.h"
#include "../Rendering/ModelJob.h"
#include "../Rendering/Camera.h"
#include "../Rendering/ESetCamera.h"

class EditorWidgetRenderSystem : public ImpureSystem
{
public:
    EditorWidgetRenderSystem(SystemParams params, IRenderer* renderer, RenderFrame* renderFrame);

    virtual void Update(double dt) override;

private:
    IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    Camera* m_EditorCamera;
    EntityWrapper m_CurrentCamera = EntityWrapper::Invalid;

    EventRelay<EditorWidgetRenderSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(Events::SetCamera& e);
};

#endif