#include "Rendering/RenderSystem.h"
#include "Rendering/DebugCameraInputController.h"

RenderSystem::RenderSystem(EventBroker* eventBrokerer, RenderQueueCollection* renderQueues)
    :ImpureSystem(eventBrokerer)
{
    m_RenderQueues = renderQueues;
    Initialize();
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &RenderSystem::OnSetCamera);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &RenderSystem::OnInputCommand);
}


bool RenderSystem::OnSetCamera(const Events::SetCamera &event)
{
    m_CurrentCamera = event.Entity;
    return true;
}

void RenderSystem::SwitchCamera(EntityID entity)
{
    m_CurrentCamera = entity;
    m_SwitchCamera = false;
    LOG_INFO("Switched to camera %i", m_CurrentCamera);
}

void RenderSystem::Initialize()
{
    
}

void RenderSystem::UpdateViewMatrix(ComponentWrapper& cameraTransform)
{
    glm::quat orientation = glm::quat((glm::vec3)cameraTransform["Orientation"]);
    glm::vec3 position = cameraTransform["Position"];

    m_ViewMatrix = glm::toMat4(glm::inverse(orientation)) * glm::translate(-position);
}

void RenderSystem::UpdateProjectionMatrix(ComponentWrapper& cameraComponent)
{
    double fov = (double&)cameraComponent["FOV"];
    double aspectRatio = (double&)cameraComponent["AspectRatio"];
    double nearClip = (double&)cameraComponent["NearClip"];
    double farClip = (double&)cameraComponent["FarClip"];

    double fovY = atan(tan(glm::radians(fov)/2.0) * aspectRatio) * 2.0;
    m_ProjectionMatrix = glm::perspective(fovY, aspectRatio, nearClip, farClip);
    
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


glm::mat4 RenderSystem::ModelMatrix(World* world, EntityID entity)
{
    glm::vec3 position = AbsolutePosition(world, entity);
    glm::quat orientation = AbsoluteOrientation(world, entity);
    glm::vec3 scale = AbsoluteScale(world, entity);

    glm::mat4 modelMatrix = glm::translate(glm::mat4(), position) * glm::toMat4(orientation) * glm::scale(scale);
    return modelMatrix;
}

void RenderSystem::FillModels(World* world, RenderQueue* renderQueue)
{
    auto models = world->GetComponents("Model");
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
                job.Matrix = m_ProjectionMatrix * m_ViewMatrix * (model->m_Matrix * ModelMatrix(world, modelC.EntityID));
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
                job.Matrix = m_ProjectionMatrix * m_ViewMatrix * (model->m_Matrix * ModelMatrix(world, modelC.EntityID));
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
            firstPersonInputController.SetPosition((glm::vec3)cameraTransform["Position"]);
        }
    }
    
    if(world->HasComponent(m_CurrentCamera, "Camera") && world->HasComponent(m_CurrentCamera, "Transform")) {

        ComponentWrapper& cameraComponent = world->GetComponent(m_CurrentCamera, "Camera");
        ComponentWrapper& cameraTransform = world->GetComponent(m_CurrentCamera, "Transform");

        firstPersonInputController.Update(dt);
        (glm::vec3&)cameraTransform["Orientation"] = glm::eulerAngles(firstPersonInputController.Orientation());
        (glm::vec3&)cameraTransform["Position"] = firstPersonInputController.Position();

       
        UpdateProjectionMatrix(cameraComponent);
        UpdateViewMatrix(cameraTransform);
    } else {
        m_ProjectionMatrix = glm::perspective(glm::radians(40.f), 1.77f, 0.05f, 5000.f);
        m_ViewMatrix = glm::mat4();

        auto cameras = world->GetComponents("Camera");

        if (cameras != nullptr) {
            ComponentWrapper& cameraC = *cameras->begin();
            m_CurrentCamera = cameraC.EntityID;
        }
    }

    m_RenderQueues->Clear();
    FillModels(world, &m_RenderQueues->Forward);
}
