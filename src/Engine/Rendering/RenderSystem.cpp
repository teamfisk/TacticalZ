#include "Rendering/RenderSystem.h"

RenderSystem::RenderSystem(EventBroker* eventBroker, const IRenderer* renderer, RenderFrame* renderFrame)
    : System(eventBroker)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
{
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &RenderSystem::OnSetCamera);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &RenderSystem::OnInputCommand);

    m_Camera = new Camera((float)m_Renderer->Resolution().Width / m_Renderer->Resolution().Height, glm::radians(45.f), 0.01f, 5000.f);
}

RenderSystem::~RenderSystem()
{
    delete m_Camera;
}

bool RenderSystem::OnSetCamera(Events::SetCamera& e)
{
    ComponentWrapper cTransform = e.CameraEntity["Transform"];
    ComponentWrapper cCamera = e.CameraEntity["Camera"];
    m_Camera->SetFOV((double)cCamera["FOV"]);
    m_Camera->SetNearClip((double)cCamera["NearClip"]);
    m_Camera->SetFarClip((double)cCamera["FarClip"]);
    m_Camera->SetPosition(cTransform["Position"]);
    m_Camera->SetOrientation(glm::quat((const glm::vec3&)cTransform["Orientation"]));
    m_CurrentCamera = e.CameraEntity;
    return true;
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
    return false;
}

void RenderSystem::Update(World* world, double dt)
{
    m_World = world;
    m_EventBroker->Process<RenderSystem>();

    if (m_CurrentCamera) {
        ComponentWrapper cameraTransform = m_CurrentCamera["Transform"];
        m_Camera->SetPosition(cameraTransform["Position"]);
        m_Camera->SetOrientation(glm::quat((const glm::vec3&)cameraTransform["Orientation"]));
    }
    //Only supports opaque geometry atm

    RenderScene scene;
    scene.Camera = m_Camera;
    scene.Viewport = Rectangle(1280, 720);
    fillModels(scene.ForwardJobs, world);
    fillLight(scene.PointLightJobs, world);
    m_RenderFrame->Add(scene);
   
}