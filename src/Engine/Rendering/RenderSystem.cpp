#include "Rendering/RenderSystem.h"

RenderSystem::RenderSystem(EventBroker* eventBrokerer, const IRenderer* renderer, RenderFrame* renderFrame) :ImpureSystem(eventBrokerer)
{
    m_Renderer = renderer;
    m_RenderFrame = renderFrame;
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &RenderSystem::OnSetCamera);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &RenderSystem::OnInputCommand);

    m_Camera = new Camera((float)m_Renderer->Resolution().Width / m_Renderer->Resolution().Height, glm::radians(45.f), 0.01f, 5000.f);
    m_DebugCameraInputController = new DebugCameraInputController<RenderSystem>(eventBrokerer, -1);
}

RenderSystem::~RenderSystem()
{
    delete m_Camera;
    delete m_DebugCameraInputController;
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

        if (m_CurrentCamera != EntityID_Invalid) {
            if (m_World->HasComponent(m_CurrentCamera, "Model")) {
                m_World->GetComponent(m_CurrentCamera, "Model")["Visible"] = true;
            }
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

void RenderSystem::updateProjectionMatrix(ComponentWrapper& cameraComponent)
{
    double fov = cameraComponent["FOV"];
    double aspectRatio = (float)m_Renderer->Resolution().Width / m_Renderer->Resolution().Height;
    double nearClip = cameraComponent["NearClip"];
    double farClip = cameraComponent["FarClip"];

    m_Camera->SetFOV(glm::radians(fov));
    m_Camera->SetAspectRatio(aspectRatio);
    m_Camera->SetNearClip(nearClip);
    m_Camera->SetFarClip(farClip);
    m_Camera->UpdateProjectionMatrix();
}

void RenderSystem::fillModels(std::list<std::shared_ptr<RenderJob>>& jobs, World* world)
{
    auto models = world->GetComponents("Model");
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

        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(resource);
        } catch (const Resource::StillLoadingException&) {
            //continue;
            model = ResourceManager::Load<::Model>("Models/Core/UnitRaptor.obj");
        } catch (const std::exception&) {
            try {
                model = ResourceManager::Load<::Model>("Models/Core/Error.obj");
            } catch (const std::exception&) {
                continue;
            }
        }

        glm::mat4 modelMatrix = Transform::ModelMatrix(modelComponent.EntityID, world);
        for (auto matGroup : model->MaterialGroups()) {
            std::shared_ptr<ModelJob> modelJob = std::shared_ptr<ModelJob>(new ModelJob(model, m_Camera, modelMatrix, matGroup, modelComponent, world));
            jobs.push_back(modelJob);
        }
    }
}


void RenderSystem::fillLight(std::list<std::shared_ptr<RenderJob>>& jobs, World* world)
{
    auto pointLights = world->GetComponents("PointLight");
    if (pointLights == nullptr) {
        return;
    }

    for (auto& pointlightC : *pointLights) {
        bool visible = pointlightC["Visible"];
        if (!visible) {
            continue;
        }
        auto transformC = world->GetComponent(pointlightC.EntityID, "Transform");
        if (&transformC == nullptr) {
            return;
        }

        std::shared_ptr<PointLightJob> pointLightJob = std::shared_ptr<PointLightJob>(new PointLightJob(transformC, pointlightC, m_World));
        jobs.push_back(pointLightJob);
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

    RenderScene scene;
    scene.Camera = m_Camera;
    scene.Viewport = Rectangle(1280, 720);
    fillModels(scene.ForwardJobs, world);
    fillLight(scene.PointLightJobs, world);
    m_RenderFrame->Add(scene);
   
}

void RenderSystem::updateCamera(World* world, double dt)
{
    if (m_SwitchCamera) {
        auto cameras = world->GetComponents("Camera");
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

        m_DebugCameraInputController->SetOrientation(glm::quat((glm::vec3)cameraTransform["Orientation"]));
        m_DebugCameraInputController->SetPosition(cameraTransform["Position"]);
    }

    if (m_World->ValidEntity(m_CurrentCamera)) {
        if (world->HasComponent(m_CurrentCamera, "Camera") && world->HasComponent(m_CurrentCamera, "Transform")) {
            ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
            ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

            m_DebugCameraInputController->Update(dt);
            (glm::vec3&)cameraTransform["Orientation"] = glm::eulerAngles(m_DebugCameraInputController->Orientation());
            (glm::vec3&)cameraTransform["Position"] = m_DebugCameraInputController->Position();

            glm::vec3 position = Transform::AbsolutePosition(world, m_CurrentCamera);
            glm::quat orientation = Transform::AbsoluteOrientation(world, m_CurrentCamera);

            m_Camera->SetPosition(position);
            m_Camera->SetOrientation(orientation);

            updateProjectionMatrix(cameraComponent);

        }
    } else {
        m_Camera = m_Camera;

        auto cameras = world->GetComponents("Camera");
        if (cameras != nullptr) {
            if (cameras->begin() != cameras->end()) {
                ComponentWrapper& cameraC = *cameras->begin();
                switchCamera(cameraC.EntityID);

                ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
                ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

                m_DebugCameraInputController->SetOrientation(glm::quat((glm::vec3)cameraTransform["Orientation"]));
                m_DebugCameraInputController->SetPosition(cameraTransform["Position"]);
            }
        }
    }

    m_Camera->UpdateViewMatrix();
}