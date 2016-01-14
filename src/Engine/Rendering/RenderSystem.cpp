#include "Rendering/RenderSystem.h"
#include "Rendering/DebugCameraInputController.h"

RenderSystem::RenderSystem(EventBroker* eventBrokerer, const IRenderer* renderer, RenderFrame* renderFrame) :ImpureSystem(eventBrokerer)
{
    m_Renderer = renderer;
    m_RenderFrame = renderFrame;
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

        Model* model = ResourceManager::Load<::Model>(resource);
        if (model == nullptr) {
            model = ResourceManager::Load<::Model>("Models/Core/Error.obj");
        }

        glm::mat4 modelMatrix = Transform::ModelMatrix(modelComponent.EntityID, world);
        
        for (auto texGroup : model->TextureGroups) {
            std::shared_ptr<ModelJob> modelJob = std::shared_ptr<ModelJob>(new ModelJob(model, m_Camera, modelMatrix, texGroup, modelComponent, world));
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

    RenderScene rs;
    rs.Camera = m_Camera;
    rs.Viewport = Rectangle(1280, 720);
    fillModels(rs.ForwardJobs, world);
    m_RenderFrame->Add(rs);
   
}

void RenderSystem::updateCamera(World* world, double dt)
{
    
        static DebugCameraInputController<RenderSystem> firstPersonInputController(m_EventBroker, -1);

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

            firstPersonInputController.SetOrientation(glm::quat((glm::vec3)cameraTransform["Orientation"]));
            firstPersonInputController.SetPosition(cameraTransform["Position"]);
            
        }

        if (m_World->ValidEntity(m_CurrentCamera)) {
            if (world->HasComponent(m_CurrentCamera, "Camera") && world->HasComponent(m_CurrentCamera, "Transform")) {
                ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
                ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

                firstPersonInputController.Update(dt);
                (glm::vec3&)cameraTransform["Orientation"] = glm::eulerAngles(firstPersonInputController.Orientation());
                (glm::vec3&)cameraTransform["Position"] = firstPersonInputController.Position();

                glm::vec3 position = Transform::AbsolutePosition(world, m_CurrentCamera);
                glm::quat orientation = Transform::AbsoluteOrientation(world, m_CurrentCamera);

                m_Camera->SetPosition(position);
                m_Camera->SetOrientation(orientation);

                updateProjectionMatrix(cameraComponent);
                
            }
        } else {
            m_Camera = m_DefaultCamera;

            auto cameras = world->GetComponents("Camera");
            if (cameras != nullptr) {
                if (cameras->begin() != cameras->end()) {
                    ComponentWrapper& cameraC = *cameras->begin();
                    switchCamera(cameraC.EntityID);

                    ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
                    ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

                    firstPersonInputController.SetOrientation(glm::quat((glm::vec3)cameraTransform["Orientation"]));
                    firstPersonInputController.SetPosition(cameraTransform["Position"]);
                }
            }
        }

        m_Camera->UpdateViewMatrix();
}

