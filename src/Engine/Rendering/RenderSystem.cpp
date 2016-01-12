#include "Rendering/RenderSystem.h"
#include "Rendering/DebugCameraInputController.h"

RenderSystem::RenderSystem(EventBroker* eventBrokerer, const IRenderer* renderer, RenderFrame* renderFrame) :ImpureSystem(eventBrokerer)
{
    m_Renderer = renderer;
    m_RenderFrame = renderFrame;
    initialize();
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &RenderSystem::OnSetCamera);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &RenderSystem::OnInputCommand);

    m_DefaultCamera = new Camera((float)m_Renderer->Resolution().Width / m_Renderer->Resolution().Height, glm::radians(45.f), 0.01f, 5000.f);
    m_DefaultCamera->SetPosition(glm::vec3(0, 0, 10));
    if (m_Camera == nullptr) {
        m_Camera = m_DefaultCamera;
    }
}

bool RenderSystem::OnSetCamera(const Events::SetCamera &event)
{
    auto cameras = m_World->GetComponents("Camera");

    if (cameras != nullptr) {
        for (auto it = cameras->begin(); it != cameras->end(); it++) {
            if ((std::string)(*it)["Name"] == event.Name) {
                switchCamera((*it).EntityID);
            }
        }
    }
    return true;
}

void RenderSystem::switchCamera(EntityID entity)
{
    if(m_World->HasComponent(entity, "Camera")) {
        if (m_World->HasComponent(m_CurrentCamera, "Model")) {
            m_World->GetComponent(m_CurrentCamera, "Model")["Visible"] = true;
        }

        if (m_World->HasComponent(entity, "Model")) {
            m_World->GetComponent(entity, "Model")["Visible"] = false;
        }
        m_CurrentCamera = entity;
        m_SwitchCamera = false;

    } else {
        LOG_ERROR("Entity %i does not have a CameraComponent", entity);
        m_SwitchCamera = false;
    }
}

void RenderSystem::initialize()
{
    
}

void RenderSystem::updateProjectionMatrix(ComponentWrapper& cameraComponent)
{
    double fov = cameraComponent["FOV"];
    double aspectRatio = m_Renderer->Resolution().Width / m_Renderer->Resolution().Height;
    double nearClip = cameraComponent["NearClip"];
    double farClip = cameraComponent["FarClip"];

    double fovY = atan(tan(glm::radians(fov)/2.0) * aspectRatio) * 2.0;
    m_ProjectionMatrix = glm::perspective(fovY, aspectRatio, nearClip, farClip);

    m_Camera->SetFOV(fovY);
    m_Camera->SetAspectRatio(aspectRatio);
    m_Camera->SetNearClip(nearClip);
    m_Camera->SetFarClip(farClip);
    m_Camera->UpdateProjectionMatrix();
}

glm::vec3 RenderSystem::AbsolutePosition(World* world, EntityID entity)
{
    glm::vec3 position;

    do {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        EntityID parent = world->GetParent(entity);
        if (parent != 0) {
            position += AbsoluteOrientation(world, parent) * (glm::vec3)transform["Position"];
        } else {
            position += (glm::vec3)transform["Position"];
        }
        entity = parent;
    } while (entity != 0);

    return position;
}

glm::quat RenderSystem::AbsoluteOrientation(World* world, EntityID entity)
{
    glm::quat orientation;

    do {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        orientation = glm::quat((glm::vec3)transform["Orientation"]) * orientation;
        entity = world->GetParent(entity);
    } while (entity != 0);

    return orientation;
}

glm::vec3 RenderSystem::AbsoluteScale(World* world, EntityID entity)
{
    glm::vec3 scale(1.f);

    do {
        ComponentWrapper transform = world->GetComponent(entity, "Transform");
        scale *= (glm::vec3)transform["Scale"];
        entity = world->GetParent(entity);
    } while (entity != 0);

    return scale;
}

glm::mat4 RenderSystem::ModelMatrix(EntityID entity)
{
    glm::vec3 position = AbsolutePosition(m_World, entity);
    glm::quat orientation = AbsoluteOrientation(m_World, entity);
    glm::vec3 scale = AbsoluteScale(m_World, entity);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation) * glm::scale(scale);
    return modelMatrix;
}

void RenderSystem::fillModels(std::list<std::shared_ptr<RenderJob>>& jobs)
{
    auto models = m_World->GetComponents("Model");
    if (models == nullptr) {
        return;
    }

    for (auto& modelComponent : *models) {
        bool visible = modelComponent["Visible"];
        if (!visible) {
            continue;
        }
        std::string resource = modelComponent["Resource"];
        if (resource.empty()) {
            continue;
        }

        Model* model = ResourceManager::Load<::Model>(resource);
        if (model == nullptr) {
            model = ResourceManager::Load<::Model>("Models/Core/Error.obj");
        }

        glm::mat4 modelMatrix = ModelMatrix(modelComponent.EntityID);
        glm::mat4 matrix = m_Camera->ProjectionMatrix() * m_Camera->ViewMatrix() * (model->m_Matrix * modelMatrix);
        
        for (auto texGroup : model->TextureGroups) {
            std::shared_ptr<ModelJob> modelJob = std::shared_ptr<ModelJob>(new ModelJob(model, m_Camera, matrix, texGroup, modelComponent));
            jobs.push_back(modelJob);
        }
    }
}

bool RenderSystem::OnInputCommand(const Events::InputCommand& e)
{
    if (e.Command == "SwitchCamera" && e.Value > 0) {
        m_SwitchCamera = true;
        return true;
    } else {
        return false;
    }
}

void RenderSystem::Update(World* world, double dt)
{
    m_World = world;
    m_EventBroker->Process<RenderSystem>();

    updateCamera(world, dt);

    //Only supports opaque geometry atm
    m_RenderFrame->Clear();
    RenderScene* rs = new RenderScene();
    rs->Camera = m_Camera;
    rs->ViewPort = Rectangle(1280, 720);
    fillModels(rs->ForwardJobs);
    m_RenderFrame->Add(*rs);
    delete rs;
}

void RenderSystem::updateCamera(World* world, double dt)
{
    static DebugCameraInputController<RenderSystem> firstPersonInputController(m_EventBroker, -1);

    if (m_SwitchCamera) {
        auto cameras = world->GetComponents("Camera");

        if (cameras != nullptr) {
            for (auto it = cameras->begin(); it != cameras->end(); it++) {
                if ((*it).EntityID == m_CurrentCamera) {
                    it++;
                    if (it != cameras->end()) {
                        switchCamera((*it).EntityID);
                    } else {
                        switchCamera((*cameras->begin()).EntityID);
                    }
                    break;
                }
            }
            ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
            ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

            firstPersonInputController.SetOrientation(glm::quat((glm::vec3)cameraTransform["Orientation"]));
            firstPersonInputController.SetPosition(cameraTransform["Position"]);
        }
    }

    if (world->HasComponent(m_CurrentCamera, "Camera") && world->HasComponent(m_CurrentCamera, "Transform")) {
        ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
        ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

        if (world->GetParent(m_CurrentCamera) == 1) { // world is entity 1, is this ok?
            firstPersonInputController.Update(dt);
            (glm::vec3&)cameraTransform["Orientation"] = glm::eulerAngles(firstPersonInputController.Orientation());
            (glm::vec3&)cameraTransform["Position"] = firstPersonInputController.Position();
        }

        glm::vec3 position = AbsolutePosition(world, m_CurrentCamera);
        glm::quat orientation = AbsoluteOrientation(world, m_CurrentCamera);

        m_Camera->SetPosition(position);
        m_Camera->SetOrientation(orientation);

        updateProjectionMatrix(cameraComponent);
        m_Camera->UpdateViewMatrix();
    } else {
        m_Camera = m_DefaultCamera;

        auto cameras = world->GetComponents("Camera");
        if (cameras != nullptr) {
            ComponentWrapper& cameraC = *cameras->begin();
            switchCamera(cameraC.EntityID);
            world->GetComponent(m_CurrentCamera, "Model")["Visible"] = false;
        }
    }
}

