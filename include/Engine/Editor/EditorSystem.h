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
#include "../Core/EntityFileWriter.h"
#include "EditorGUI.h"
#include "EditorStats.h"

class EditorSystem : public ImpureSystem
{
public:
    EditorSystem(World* world, EventBroker* eventBroker, IRenderer* renderer, RenderFrame* renderFrame);
    ~EditorSystem();

    void Update(double dt);

private:
    IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    World* m_EditorWorld;
    SystemPipeline* m_EditorWorldSystemPipeline;
    Camera* m_EditorCamera;
    EntityWrapper m_Camera = EntityWrapper::Invalid;
    DebugCameraInputController<EditorSystem>* m_DebugCameraInputController;
    EditorGUI* m_EditorGUI;
    EditorStats* m_EditorStats;

    // State
    EditorGUI::WidgetMode m_WidgetMode = EditorGUI::WidgetMode::Translate;
    EntityWrapper m_Widget = EntityWrapper::Invalid;
    EntityWrapper m_CurrentSelection = EntityWrapper::Invalid;

    // Utility functions
    EntityWrapper importEntity(EntityWrapper parent, boost::filesystem::path filePath);
    void setWidgetMode(EditorGUI::WidgetMode mode);

    // GUI callbacks
    void OnEntitySelected(EntityWrapper entity);
    void OnEntitySave(EntityWrapper entity, boost::filesystem::path filePath);
    EntityWrapper OnEntityCreate(EntityWrapper parent);
    void OnEntityDelete(EntityWrapper entity);
    void OnEntityChangeParent(EntityWrapper entity, EntityWrapper parent);
    void OnEntityChangeName(EntityWrapper entity, const std::string& name);
    void OnComponentAttach(EntityWrapper entity, const std::string& componentType);
    void OnComponentDelete(EntityWrapper entity, const std::string& componentType);
};