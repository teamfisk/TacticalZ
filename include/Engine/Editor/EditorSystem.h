#include "../Core/System.h"
#include "../Rendering/IRenderer.h"
#include "../Rendering/Camera.h"
#include "../Rendering/ESetCamera.h"
#include "../Core/World.h"
#include "../Core/SystemPipeline.h"
#include "../Core/ResourceManager.h"
#include "../Core/EntityFilePreprocessor.h"
#include "../Core/EntityFileParser.h"
#include "../Core/EntityFileWriter.h"
#include "../Core/EMousePress.h"
#include "../Input/EInputCommand.h"
#include "EditorGUI.h"
#include "EditorStats.h"
#include "EditorCameraInputController.h"

class EditorSystem : public ImpureSystem
{
public:
    EditorSystem(SystemParams params, IRenderer* renderer, RenderFrame* renderFrame);
    ~EditorSystem();

    void Update(double dt);

    void Enable();
    void Disable();

private:
    IRenderer* m_Renderer;
    RenderFrame* m_RenderFrame;
    World* m_EditorWorld;
    SystemPipeline* m_EditorWorldSystemPipeline;
    //Camera* m_EditorCamera;
    EntityWrapper m_EditorCamera = EntityWrapper::Invalid;
    EntityWrapper m_ActualCamera = EntityWrapper::Invalid;
    EditorCameraInputController<EditorSystem>* m_EditorCameraInputController;
    EditorGUI* m_EditorGUI;
    EditorStats* m_EditorStats;
    std::vector<World> m_UndoLevels;

    // State
    double m_LastTime = 0.f;
    bool m_Enabled = true;
    EditorGUI::WidgetMode m_WidgetMode = EditorGUI::WidgetMode::Translate;
    EditorGUI::WidgetSpace m_WidgetSpace = EditorGUI::WidgetSpace::Global;
    EntityWrapper m_Widget = EntityWrapper::Invalid;
    EntityWrapper m_CurrentSelection = EntityWrapper::Invalid;
    bool m_SaveUndoLevel = false; // Only save undo state once per update

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
    EntityWrapper OnEntityPaste(EntityWrapper entityToCopy, EntityWrapper parent);
    void OnComponentAttach(EntityWrapper entity, const std::string& componentType);
    void OnComponentDelete(EntityWrapper entity, const std::string& componentType);
    void OnWidgetSpace(EditorGUI::WidgetSpace widgetSpace);
    void OnDirty(EntityWrapper entity);
    void OnUndo();

    // Events
    EventRelay<EditorSystem, Events::MousePress> m_EMousePress;
    bool OnMousePress(const Events::MousePress& e);
    EventRelay<EditorSystem, Events::WidgetDelta> m_EWidgetDelta;
    bool OnWidgetDelta(const Events::WidgetDelta& e);
    EventRelay<EditorSystem, Events::InputCommand> m_EInputCommand;
    bool OnInputCommand(const Events::InputCommand& e);
    EventRelay<EditorSystem, Events::SetCamera> m_ESetCamera;
    bool OnSetCamera(const Events::SetCamera& e);
};