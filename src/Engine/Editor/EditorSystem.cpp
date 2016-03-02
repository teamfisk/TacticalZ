#include "Editor/EditorSystem.h"
#include "Core/UniformScaleSystem.h"
#include "Editor/EditorRenderSystem.h"
#include "Editor/EditorWidgetSystem.h"

EditorSystem::EditorSystem(SystemParams params, IRenderer* renderer, RenderFrame* renderFrame) 
    : System(params)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
{
    m_EditorWorld = new World();
    m_EditorWorldSystemPipeline = new SystemPipeline(m_EditorWorld, m_EventBroker, IsClient, IsServer);
    m_EditorWorldSystemPipeline->AddSystem<UniformScaleSystem>(0);
    m_EditorWorldSystemPipeline->AddSystem<EditorWidgetSystem>(0, m_Renderer);
    m_EditorWorldSystemPipeline->AddSystem<EditorRenderSystem>(1, m_Renderer, m_RenderFrame);
    
    m_EditorCamera = importEntity(EntityWrapper(m_EditorWorld, EntityID_Invalid), "Schema/Entities/Empty.xml");
    m_ActualCamera = m_EditorCamera;
    m_EditorWorld->AttachComponent(m_EditorCamera.ID, "Transform");
	auto cCamera = m_EditorWorld->AttachComponent(m_EditorCamera.ID, "Camera");
	// TOBIAS TVINGADE MIG ATT HÅRDKODA
	(double&)cCamera["FarClip"] = 400.0;

    m_EditorCameraInputController = new EditorCameraInputController<EditorSystem>(m_EventBroker, -1);

    m_EditorGUI = new EditorGUI(m_World, m_EventBroker);
    m_EditorGUI->SetEntitySelectedCallback(std::bind(&EditorSystem::OnEntitySelected, this, std::placeholders::_1));
    m_EditorGUI->SetEntityImportCallback(std::bind(&EditorSystem::importEntity, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetEntitySaveCallback(std::bind(&EditorSystem::OnEntitySave, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetEntityCreateCallback(std::bind(&EditorSystem::OnEntityCreate, this, std::placeholders::_1));
    m_EditorGUI->SetEntityDeleteCallback(std::bind(&EditorSystem::OnEntityDelete, this, std::placeholders::_1));
    m_EditorGUI->SetEntityChangeParentCallback(std::bind(&EditorSystem::OnEntityChangeParent, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetEntityChangeNameCallback(std::bind(&EditorSystem::OnEntityChangeName, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetEntityPasteCallback(std::bind(&EditorSystem::OnEntityPaste, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetComponentAttachCallback(std::bind(&EditorSystem::OnComponentAttach, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetComponentDeleteCallback(std::bind(&EditorSystem::OnComponentDelete, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetWidgetModeCallback(std::bind(&EditorSystem::setWidgetMode, this, std::placeholders::_1));
    m_EditorGUI->SetWidgetSpaceCallback(std::bind(&EditorSystem::OnWidgetSpace, this, std::placeholders::_1));

    EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &EditorSystem::OnMousePress);
    EVENT_SUBSCRIBE_MEMBER(m_EWidgetDelta, &EditorSystem::OnWidgetDelta);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &EditorSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &EditorSystem::OnSetCamera);

    m_EditorStats = new EditorStats();

    m_Enabled = ResourceManager::Load<ConfigFile>("Config.ini")->Get<bool>("Debug.EditorEnabled", false);
    if (m_Enabled) {
        Enable();
    } else {
        Disable();
    }
}

EditorSystem::~EditorSystem()
{
    delete m_EditorStats;
    delete m_EditorGUI;
    delete m_EditorCameraInputController;
    delete m_EditorWorldSystemPipeline;
    delete m_EditorWorld;
}

void EditorSystem::Update(double dt)
{
    double now = glfwGetTime();
    double actualDelta = now - m_LastTime;
    m_LastTime = now;

    if (m_Enabled) {
        m_EventBroker->Process<EditorGUI>();
        m_EditorGUI->Draw();
        m_EditorStats->Draw(actualDelta);

        if (m_CurrentSelection.Valid() && m_Widget.Valid()) {
            (glm::vec3&)m_Widget["Transform"]["Position"] = Transform::AbsolutePosition(m_CurrentSelection);
            if (m_WidgetSpace == EditorGUI::WidgetSpace::Local) {
                (glm::vec3&)m_Widget["Transform"]["Orientation"] = Transform::AbsoluteOrientationEuler(m_CurrentSelection);
            } else {
                (glm::vec3&)m_Widget["Transform"]["Orientation"] = glm::vec3(0, 0, 0);
            }
        }

        m_EditorWorldSystemPipeline->Update(actualDelta);

        ComponentWrapper& cameraTransform = m_EditorCamera["Transform"];
        glm::vec3& ori = cameraTransform["Orientation"];
        ori.x = m_EditorCameraInputController->Rotation().x;
        ori.y = m_EditorCameraInputController->Rotation().y;
        glm::vec3& pos = cameraTransform["Position"];
        pos += m_EditorCameraInputController->Movement() * glm::inverse(glm::quat(ori)) * (float)actualDelta;
    }
}

void EditorSystem::Enable()
{
    m_EditorCameraInputController->Enable();

    m_EventBroker->Publish(Events::UnlockMouse());

    // Enable editor camera
    Events::SetCamera eSetCamera;
    eSetCamera.CameraEntity = m_EditorCamera;
    m_EventBroker->Publish(eSetCamera);
    if (m_ActualCamera.Valid()) {
        (glm::vec3&)m_EditorCamera["Transform"]["Position"] = Transform::AbsolutePosition(m_ActualCamera);
    }

    // Pause the world we're editing
    //Events::Pause ePause;
    //ePause.World = m_World;
    //m_EventBroker->Publish(ePause);

    m_Enabled = true;
}

void EditorSystem::Disable()
{
    m_EditorCameraInputController->Disable();
    m_EventBroker->Publish(Events::LockMouse());
    Events::SetCamera e;
    e.CameraEntity = m_ActualCamera;
    m_EventBroker->Publish(e);
    m_Enabled = false;
}

void EditorSystem::OnEntitySelected(EntityWrapper entity)
{
    m_CurrentSelection = entity;
    setWidgetMode(m_WidgetMode);
}

void EditorSystem::OnEntitySave(EntityWrapper entity, boost::filesystem::path filePath)
{
    EntityFileWriter writer(filePath);
    writer.WriteEntity(entity.World, entity.ID);
}

EntityWrapper EditorSystem::OnEntityCreate(EntityWrapper parent)
{
    EntityID entity = parent.World->CreateEntity(parent.ID);
    parent.World->AttachComponent(entity, "Transform");
    return EntityWrapper(parent.World, entity);
}

void EditorSystem::OnEntityDelete(EntityWrapper entity)
{
    if (entity.Valid()) {
        entity.World->DeleteEntity(entity.ID);
    }
}

void EditorSystem::OnEntityChangeParent(EntityWrapper entity, EntityWrapper parent)
{
    if (entity.Valid()) {
        entity.World->SetParent(entity.ID, parent.ID);
    }
}

void EditorSystem::OnEntityChangeName(EntityWrapper entity, const std::string& name)
{
    if (entity.Valid()) {
        entity.World->SetName(entity.ID, name);
    }
}

EntityWrapper EditorSystem::OnEntityPaste(EntityWrapper entityToCopy, EntityWrapper parent)
{
    return entityToCopy.Clone(parent);
}

void EditorSystem::OnComponentAttach(EntityWrapper entity, const std::string& componentType)
{
    if (entity.Valid()) {
        entity.World->AttachComponent(entity.ID, componentType);
    }
}

void EditorSystem::OnComponentDelete(EntityWrapper entity, const std::string& componentType)
{
    if (entity.Valid()) {
        entity.World->DeleteComponent(entity.ID, componentType);
    }
}

void EditorSystem::OnWidgetSpace(EditorGUI::WidgetSpace widgetSpace)
{
    m_WidgetSpace = widgetSpace;
}

bool EditorSystem::OnMousePress(const Events::MousePress& e)
{
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse && !io.WantCaptureKeyboard && e.Button == GLFW_MOUSE_BUTTON_1) {
        PickData pick = m_Renderer->Pick(glm::vec2(e.X, e.Y));
        if (pick.World == m_World) {
            m_CurrentSelection = EntityWrapper(m_World, pick.Entity);
            m_EditorGUI->SelectEntity(m_CurrentSelection);
        }
    }
    return true;
}

bool EditorSystem::OnWidgetDelta(const Events::WidgetDelta& e)
{
    if (m_CurrentSelection.Valid()) {
        if (m_WidgetSpace == EditorGUI::WidgetSpace::Global) {
            glm::quat parentOrientation;
            EntityWrapper parent = m_CurrentSelection.Parent();
            if (parent.Valid()) {
                parentOrientation = glm::inverse(Transform::AbsoluteOrientation(parent));
            }
            (glm::vec3&)m_CurrentSelection["Transform"]["Position"] += parentOrientation * e.Translation;
        } else if (m_WidgetSpace == EditorGUI::WidgetSpace::Local) {
            glm::quat selectionOri = glm::quat((glm::vec3)m_CurrentSelection["Transform"]["Orientation"]);
            glm::vec3 localTranslation = selectionOri * e.Translation;
            (glm::vec3&)m_CurrentSelection["Transform"]["Position"] += localTranslation;
        }
        m_EditorGUI->SetDirty(m_CurrentSelection);
    }
    return true;
}

bool EditorSystem::OnInputCommand(const Events::InputCommand& e)
{
    if (e.PlayerID != -1) {
        return false;
    }

    if (e.Command == "ToggleEditor" && e.Value > 0) {
        if (m_Enabled) {
            Disable();
        } else {
            Enable();
        }
    }
    if (e.Command == "PerformanceTimingResetAllTimers" && e.Value > 0) {
        PerformanceTimer::ResetAllTimers();
    }
    if (e.Command == "PerformanceTimingCreateExcelData" && e.Value > 0) {
        PerformanceTimer::CreateExcelData();
    }
    return true;
}

bool EditorSystem::OnSetCamera(const Events::SetCamera& e)
{
    if (m_Enabled && e.CameraEntity != m_EditorCamera) {
        m_ActualCamera = e.CameraEntity;
        Events::SetCamera e2;
        e2.CameraEntity = m_EditorCamera;
        //m_EventBroker->Publish(e2);
    }
    return true;
}

EntityWrapper EditorSystem::importEntity(EntityWrapper parent, boost::filesystem::path filePath)
{
    if (parent.World == nullptr) {
        LOG_ERROR("Tried to import entity \"%s\" into null world!", filePath.string().c_str());
        return EntityWrapper::Invalid;
    }

    try {
        auto entityFile = ResourceManager::Load<EntityFile>(filePath.string());
        EntityFilePreprocessor fpp(entityFile);
        fpp.RegisterComponents(parent.World);
        EntityFileParser fp(entityFile);
        EntityID newEntity = fp.MergeEntities(parent.World, parent.ID);
        return EntityWrapper(parent.World, newEntity);
    } catch (const std::exception&) {
        return EntityWrapper::Invalid;
    }
}

void EditorSystem::setWidgetMode(EditorGUI::WidgetMode mode)
{
    if (mode == m_WidgetMode && m_Widget.Valid() && m_CurrentSelection.Valid()) {
        return;
    }

    m_WidgetMode = mode;

    if (m_Widget.Valid()) {
        m_Widget.World->DeleteEntity(m_Widget.ID);
        m_Widget = EntityWrapper::Invalid;
    }

    if (!m_CurrentSelection.Valid()) {
        return;
    }

    switch (mode) {
    case EditorGUI::WidgetMode::Translate:
        m_Widget = importEntity(EntityWrapper(m_EditorWorld, EntityID_Invalid), "Schema/Entities/EditorWidgetTranslate.xml");
        break;
    case EditorGUI::WidgetMode::Rotate:
        m_Widget = importEntity(EntityWrapper(m_EditorWorld, EntityID_Invalid), "Schema/Entities/EditorWidgetRotate.xml");
        break;
    case EditorGUI::WidgetMode::Scale:
        m_Widget = importEntity(EntityWrapper(m_EditorWorld, EntityID_Invalid), "Schema/Entities/EditorWidgetScale.xml");
        break;
    }

    m_Widget["Transform"]["Position"] = Transform::AbsolutePosition(m_CurrentSelection.World, m_CurrentSelection.ID);
}
