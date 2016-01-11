#include "Rendering/RenderSystem.h"
#include "Rendering/DebugCameraInputController.h"

RenderSystem::RenderSystem(EventBroker* eventBrokerer, RenderFrame* renderFrame)
    :ImpureSystem(eventBrokerer)
{
    m_RenderFrame = renderFrame;
    Initialize();
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &RenderSystem::OnSetCamera);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &RenderSystem::OnInputCommand);

    m_DefaultCamera = new Camera(1.77f /* should be based on width and height */, glm::radians(45.f), 0.01f, 5000.f);
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
                SwitchCamera((*it).EntityID);
            }
        }
    }

    return true;
}

void RenderSystem::SwitchCamera(EntityID entity)
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

        LOG_INFO("Switched to %s", m_World->GetComponent(m_CurrentCamera, "Camera")["Name"]);
    } else {
        LOG_ERROR("Entity %i does not have a CameraComponent", entity);
        m_SwitchCamera = false;
    }
}

void RenderSystem::Initialize()
{
    
}

void RenderSystem::UpdateProjectionMatrix(ComponentWrapper& cameraComponent)
{
    double fov = cameraComponent["FOV"];
    double aspectRatio = cameraComponent["AspectRatio"];
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

void RenderSystem::FillModels(RenderQueue* renderQueue)
{
    auto models = m_World->GetComponents("Model");
    if (models == nullptr) {
        return;
    }

    for (auto& modelC : *models) {
        bool visible = modelC["Visible"];
        if (!visible) {
            continue;
        }
        std::string resource = modelC["Resource"];
        if (resource.empty()) {
            continue;
        }
        glm::vec4 color = modelC["Color"];
        Model* model = ResourceManager::Load<Model>(resource);
        if (model == nullptr) {
            model = ResourceManager::Load<Model>("Models/Core/Error.obj");
        }

        for (auto texGroup : model->TextureGroups) {

            if (color.a < 1.0f || texGroup.Transparency < 1.0f) {
                //transparent stuffs
                TransparentModelJob job;
                job.TextureID = (texGroup.Texture) ? texGroup.Texture->ResourceID : 0;
                job.DiffuseTexture = texGroup.Texture.get();
                job.NormalTexture = texGroup.NormalMap.get();
                job.SpecularTexture = texGroup.SpecularMap.get();
                job.Model = model;
                job.StartIndex = texGroup.StartIndex;
                job.EndIndex = texGroup.EndIndex;
                job.Matrix = m_Camera->ProjectionMatrix() * m_Camera->ViewMatrix() * (model->m_Matrix * ModelMatrix(modelC.EntityID));
                job.Color = color;
                job.Entity = modelC.EntityID;

                job.Depth = 10.f; //insert real viewspace depth here

                renderQueue->Add(job);
            } else {
                ModelJob job;
                job.TextureID = (texGroup.Texture) ? texGroup.Texture->ResourceID : 0;
                job.DiffuseTexture = texGroup.Texture.get();
                job.NormalTexture = texGroup.NormalMap.get();
                job.SpecularTexture = texGroup.SpecularMap.get();
                job.Model = model;
                job.StartIndex = texGroup.StartIndex;
                job.EndIndex = texGroup.EndIndex;
                job.Matrix = m_Camera->ProjectionMatrix() * m_Camera->ViewMatrix() * (model->m_Matrix * ModelMatrix(modelC.EntityID));
                job.Color = color;
                job.Entity = modelC.EntityID;

                renderQueue->Add(job);
            }
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
    static DebugCameraInputController<RenderSystem> firstPersonInputController(m_EventBroker, -1);

    if (m_SwitchCamera) {
        auto cameras = world->GetComponents("Camera");

        if (cameras != nullptr) {
            for (auto it = cameras->begin(); it != cameras->end(); it++) {
                if ((*it).EntityID == m_CurrentCamera) {
                    it++;
                    if (it != cameras->end()) {
                        SwitchCamera((*it).EntityID);
                    } else {
                        SwitchCamera((*cameras->begin()).EntityID);
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
    
    if(world->HasComponent(m_CurrentCamera, "Camera") && world->HasComponent(m_CurrentCamera, "Transform")) {
        ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
        ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

        if(world->GetParent(m_CurrentCamera) == 1) { // world is entity 1, is this ok?
            firstPersonInputController.Update(dt);
            (glm::vec3&)cameraTransform["Orientation"] = glm::eulerAngles(firstPersonInputController.Orientation());
            (glm::vec3&)cameraTransform["Position"] = firstPersonInputController.Position();
        }

        glm::vec3 position = AbsolutePosition(world, m_CurrentCamera);
        glm::quat orientation = AbsoluteOrientation(world, m_CurrentCamera);
       
        m_Camera->SetPosition(position);
        m_Camera->SetOrientation(orientation);

        UpdateProjectionMatrix(cameraComponent);
        m_Camera->UpdateViewMatrix();
    } else {
        m_Camera = m_DefaultCamera;

        auto cameras = world->GetComponents("Camera");
        if (cameras != nullptr) {
            ComponentWrapper& cameraC = *cameras->begin();
            SwitchCamera(cameraC.EntityID);
            world->GetComponent(m_CurrentCamera, "Model")["Visible"] = false;
        }
    }

    m_RenderFrame->Clear();
    RenderScene* rs = new RenderScene();
    rs->Camera = m_Camera;
    FillModels(&rs->Forward);
    m_RenderFrame->Add(*rs);
    delete rs;
}
