#include "../Core/System.h"
#include "../Rendering/IRenderer.h"
#include "../Rendering/Camera.h"
#include "../Rendering/DebugCameraInputController.h"
#include "../Rendering/ESetCamera.h"
#include "../Core/World.h"
#include "../Core/SystemPipeline.h"
#include "../Core/ResourceManager.h"
#include "../Core/EntityFilePreprocessor.h"
#include "../Core/EntityFileParser.h"

class EditorSystem : public ImpureSystem
{
public:
    EditorSystem(EventBroker* eventBroker, IRenderer* renderer, RenderFrame* renderFrame);
    ~EditorSystem();

    void Update(World* world, double dt);

private:
    IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    World* m_EditorWorld;
    SystemPipeline* m_EditorWorldSystemPipeline;
    Camera* m_EditorCamera;

    EntityWrapper m_Widget = EntityWrapper::Invalid;
    EntityWrapper m_Camera = EntityWrapper::Invalid;
    DebugCameraInputController<EditorSystem>* m_DebugCameraInputController;
};