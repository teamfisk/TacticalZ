#include "../Core/System.h"
#include "../Rendering/IRenderer.h"
#include "../Rendering/ModelJob.h"
#include "../Rendering/Camera.h"
#include "../Rendering/ESetCamera.h"

class EditorRenderSystem : public ImpureSystem
{
public:
    EditorRenderSystem(SystemParams params, IRenderer* renderer, RenderFrame* renderFrame);

    virtual void Update(double dt) override;

private:
    IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    Camera* m_Camera;
    EntityWrapper m_CurrentCamera = EntityWrapper::Invalid;

    EventRelay<EditorRenderSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(Events::SetCamera& e);

    void enqueueModel(RenderScene& scene, EntityWrapper entity, const glm::mat4& modelMatrix, const std::string& modelResource, ComponentWrapper cModel, bool wireframe);
};