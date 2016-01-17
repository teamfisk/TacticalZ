#include "Editor/EditorSystem.h"
#include "Core/UniformScaleSystem.h"
#include "Editor/EditorRenderSystem.h"

EditorSystem::EditorSystem(EventBroker* eventBroker, IRenderer* renderer, RenderFrame* renderFrame) 
    : System(eventBroker)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
{
    m_EditorWorld = new World();
    m_EditorWorldSystemPipeline = new SystemPipeline(eventBroker);
    m_EditorWorldSystemPipeline->AddSystem<UniformScaleSystem>(0);
    m_EditorWorldSystemPipeline->AddSystem<EditorRenderSystem>(1, m_Renderer, m_RenderFrame);
    
    auto widgetEntityFile = ResourceManager::Load<EntityFile>("Schema/Entities/EditorWidget.xml");
    EntityFilePreprocessor fpp(widgetEntityFile);
    fpp.RegisterComponents(m_EditorWorld);
    EntityFileParser fp(widgetEntityFile);
    EntityID widgetID = fp.MergeEntities(m_EditorWorld);
    m_Widget = EntityWrapper(m_EditorWorld, widgetID);

    m_Camera = EntityWrapper(m_EditorWorld, m_EditorWorld->CreateEntity());
    m_EditorWorld->AttachComponent(m_Camera.ID, "Transform");
    m_EditorWorld->AttachComponent(m_Camera.ID, "Camera");
    m_DebugCameraInputController = new DebugCameraInputController<EditorSystem>(m_EventBroker, -1);

    Events::SetCamera e;
    e.CameraEntity = m_Camera;
    m_EventBroker->Publish(e);
}

EditorSystem::~EditorSystem()
{
    delete m_DebugCameraInputController;
    delete m_EditorWorldSystemPipeline;
    delete m_EditorWorld;
}

void EditorSystem::Update(World* world, double dt)
{
    m_EditorWorldSystemPipeline->Update(m_EditorWorld, dt);

    m_DebugCameraInputController->Update(dt);
    m_Camera["Transform"]["Position"] = m_DebugCameraInputController->Position();
    m_Camera["Transform"]["Orientation"] = glm::eulerAngles(m_DebugCameraInputController->Orientation());
}