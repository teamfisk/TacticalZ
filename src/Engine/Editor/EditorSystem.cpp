#include "Editor/EditorSystem.h"
#include "Core/UniformScaleSystem.h"
#include "Editor/EditorRenderSystem.h"

EditorSystem::EditorSystem(World* world, EventBroker* eventBroker, IRenderer* renderer, RenderFrame* renderFrame) 
    : System(world, eventBroker)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
{
    m_EditorWorld = new World();
    m_EditorWorldSystemPipeline = new SystemPipeline(m_EditorWorld, eventBroker);
    m_EditorWorldSystemPipeline->AddSystem<UniformScaleSystem>(0);
    m_EditorWorldSystemPipeline->AddSystem<EditorRenderSystem>(1, m_Renderer, m_RenderFrame);
    
    m_Widget = importEntity(EntityWrapper(m_EditorWorld, EntityID_Invalid), "Schema/Entities/EditorWidget.xml");

    m_Camera = EntityWrapper(m_EditorWorld, m_EditorWorld->CreateEntity());
    m_EditorWorld->AttachComponent(m_Camera.ID, "Transform");
    m_EditorWorld->AttachComponent(m_Camera.ID, "Camera");
    m_DebugCameraInputController = new DebugCameraInputController<EditorSystem>(m_EventBroker, -1);

    m_EditorGUI = new EditorGUI(m_EventBroker);
    m_EditorGUI->SetEntitySelectedCallback(std::bind(&EditorSystem::OnEntitySelected, this, std::placeholders::_1));
    m_EditorGUI->SetEntityImportCallback(std::bind(&EditorSystem::importEntity, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetEntitySaveCallback(std::bind(&EditorSystem::OnEntitySave, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetEntityCreateCallback(std::bind(&EditorSystem::OnEntityCreate, this, std::placeholders::_1));
    m_EditorGUI->SetEntityDeleteCallback(std::bind(&EditorSystem::OnEntityDelete, this, std::placeholders::_1));
    m_EditorGUI->SetEntityChangeParentCallback(std::bind(&EditorSystem::OnEntityChangeParent, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetComponentAttachCallback(std::bind(&EditorSystem::OnComponentAttach, this, std::placeholders::_1, std::placeholders::_2));
    m_EditorGUI->SetComponentDeleteCallback(std::bind(&EditorSystem::OnComponentDelete, this, std::placeholders::_1, std::placeholders::_2));

    m_EditorStats = new EditorStats();

    Events::SetCamera e;
    e.CameraEntity = m_Camera;
    m_EventBroker->Publish(e);
}

EditorSystem::~EditorSystem()
{
    delete m_EditorStats;
    delete m_EditorGUI;
    delete m_DebugCameraInputController;
    delete m_EditorWorldSystemPipeline;
    delete m_EditorWorld;
}

void EditorSystem::Update(double dt)
{
    m_EditorWorldSystemPipeline->Update(dt);

    m_EditorGUI->Draw(m_World);
    m_EditorStats->Draw(dt);

    m_DebugCameraInputController->Update(dt);
    m_Camera["Transform"]["Position"] = m_DebugCameraInputController->Position();
    m_Camera["Transform"]["Orientation"] = glm::eulerAngles(m_DebugCameraInputController->Orientation());
}

void EditorSystem::OnEntitySelected(EntityWrapper entity)
{
    m_Widget["Transform"]["Position"] = Transform::AbsolutePosition(entity.World, entity.ID);
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
    entity.World->DeleteEntity(entity.ID);
}

void EditorSystem::OnEntityChangeParent(EntityWrapper entity, EntityWrapper parent)
{
    entity.World->SetParent(entity.ID, parent.ID);
}

void EditorSystem::OnComponentAttach(EntityWrapper entity, const std::string& componentType)
{
    entity.World->AttachComponent(entity.ID, componentType);
}

void EditorSystem::OnComponentDelete(EntityWrapper entity, const std::string& componentType)
{
    entity.World->DeleteComponent(entity.ID, componentType);
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