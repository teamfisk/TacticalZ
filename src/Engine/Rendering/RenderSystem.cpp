#include "Rendering/RenderSystem.h"

RenderSystem::RenderSystem(World* world, EventBroker* eventBroker, const IRenderer* renderer, RenderFrame* renderFrame) 
    : System(world, eventBroker)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
    , m_World(world)
{
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &RenderSystem::OnSetCamera);
    EVENT_SUBSCRIBE_MEMBER(m_EInputCommand, &RenderSystem::OnInputCommand);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerSpawned, &RenderSystem::OnPlayerSpawned);

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

bool RenderSystem::isChildOfACamera(EntityWrapper entity)
{
    return entity.FirstParentWithComponent("Camera").Valid();
}
bool RenderSystem::isChildOfCurrentCamera(EntityWrapper entity)
{
    return entity == m_CurrentCamera || entity.IsChildOf(m_CurrentCamera);
}

void RenderSystem::fillModels(std::list<std::shared_ptr<RenderJob>>& opaqueJobs, std::list<std::shared_ptr<RenderJob>>& transparentJobs)
{
    auto models = m_World->GetComponents("Model");
    if (models == nullptr) {
        return;
    }

    for (auto& cModel : *models) {
        bool visible = cModel["Visible"];
        if (!visible) {
            continue;
        }
        std::string resource = cModel["Resource"];
        if (resource.empty()) {
            continue;
        }

        EntityWrapper entity(m_World, cModel.EntityID);

        // Only render children of a camera if that camera is currently active
        if (isChildOfACamera(entity) && !isChildOfCurrentCamera(entity)) {
            continue;
        }

        // Hide things parented to local player if they have the HiddenFromLocalPlayer component
        if (entity.HasComponent("HiddenForLocalPlayer") && (entity == m_LocalPlayer || entity.IsChildOf(m_LocalPlayer))) {
            continue;
        }

        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(resource);
        } catch (const Resource::StillLoadingException&) {
            //continue;
            model = ResourceManager::Load<::Model>("Models/Core/UnitRaptor.mesh");
        } catch (const std::exception&) {
            try {
                model = ResourceManager::Load<::Model>("Models/Core/Error.mesh");
            } catch (const std::exception&) {
                continue;
            }
        }

        float fillPercentage = 0.f;
        glm::vec4 fillColor = glm::vec4(0);
        if(m_World->HasComponent(cModel.EntityID, "Fill")) {
            auto fillComponent = m_World->GetComponent(cModel.EntityID, "Fill");
            fillPercentage = (float)(double)fillComponent["Percentage"];
            fillColor = (glm::vec4)fillComponent["Color"];
        }

        glm::mat4 modelMatrix = Transform::ModelMatrix(cModel.EntityID, m_World);
        for (auto matGroup : model->MaterialGroups()) {
            if (m_World->HasComponent(cModel.EntityID, "ExplosionEffect")) {
                auto explosionEffectComponent = m_World->GetComponent(cModel.EntityID, "ExplosionEffect");
                std::shared_ptr<ExplosionEffectJob> explosionEffectJob = std::shared_ptr<ExplosionEffectJob>(new ExplosionEffectJob(
                    explosionEffectComponent, 
                    model, 
                    m_Camera, 
                    modelMatrix, 
                    matGroup, 
                    cModel, 
                    m_World, 
                    fillColor, 
                    fillPercentage
                ));
                if(explosionEffectJob->Color.a != 1.f || explosionEffectJob->EndColor.a != 1.f || explosionEffectJob->DiffuseColor.a != 1.f) {
                    cModel["Transparent"] = true;
                }

                if (cModel["Transparent"]) {
                    transparentJobs.push_back(explosionEffectJob);
                } else {
                    opaqueJobs.push_back(explosionEffectJob);
                }
            } else {
                std::shared_ptr<ModelJob> modelJob = std::shared_ptr<ModelJob>(new ModelJob(
                    model, 
                    m_Camera, 
                    modelMatrix, 
                    matGroup, 
                    cModel, 
                    m_World, 
                    fillColor, 
                    fillPercentage
                ));
                if (modelJob->Color.a != 1.f || modelJob->DiffuseColor.a != 1.f) {
                    cModel["Transparent"] = true;
                }
                if (cModel["Transparent"]) {
                    transparentJobs.push_back(modelJob);
                } else {
                    opaqueJobs.push_back(modelJob);
                }
            }
        }
    }
}

bool RenderSystem::OnPlayerSpawned(Events::PlayerSpawned& e)
{
    if (e.PlayerID == -1) {
        m_LocalPlayer = e.Player;
    }
    return true;
}

void RenderSystem::fillPointLights(std::list<std::shared_ptr<RenderJob>>& jobs, World* world)
{
    auto pointLights = m_World->GetComponents("PointLight");
    if (pointLights != nullptr) {
        for (auto& pointlightC : *pointLights) {
            bool visible = pointlightC["Visible"];
            if (!visible) {
                continue;
            }
            auto transformC = world->GetComponent(pointlightC.EntityID, "Transform");
            if (&transformC == nullptr) {
                continue;
            }

            std::shared_ptr<PointLightJob> pointLightJob = std::shared_ptr<PointLightJob>(new PointLightJob(transformC, pointlightC, m_World));
            jobs.push_back(pointLightJob);
        }
    }
}


void RenderSystem::fillDirectionalLights(std::list<std::shared_ptr<RenderJob>>& jobs, World* world)
{
    auto directionalLights = world->GetComponents("DirectionalLight");
    if (directionalLights != nullptr) {
        for (auto& directionalLightC : *directionalLights) {
            bool visable = directionalLightC["Visible"];
            if (!visable) {
                continue;
            }

            auto transformC = world->GetComponent(directionalLightC.EntityID, "Transform");
            if (&transformC == nullptr) {
                continue;
            }

            std::shared_ptr<DirectionalLightJob> directionalLightJob = std::shared_ptr<DirectionalLightJob>(new DirectionalLightJob(transformC, directionalLightC, m_World));
            jobs.push_back(directionalLightJob);
        }
    }
}


void RenderSystem::fillText(std::list<std::shared_ptr<RenderJob>>& jobs, World* world)
{
    auto texts = world->GetComponents("Text");
    if (texts == nullptr) {
        return;
    }

    for (auto& textComponent : *texts) {
        bool visible = textComponent["Visible"];
        if (!visible) {
            continue;
        }
        std::string resource = textComponent["Resource"];
        if (resource.empty()) {
            continue;
        }

        Font* font;
        try {
            font = ResourceManager::Load<Font>(resource);
        } catch (const std::exception&) {
            try {
                font = ResourceManager::Load<Font>("Fonts/DroidSans.ttf,16");
            } catch (const std::exception&) {
                continue;
            }
        }

        glm::mat4 modelMatrix = Transform::ModelMatrix(textComponent.EntityID, world);
        std::shared_ptr<TextJob> textJob = std::shared_ptr<TextJob>(new TextJob(modelMatrix, font, textComponent));
        jobs.push_back(textJob);

    }
}

bool RenderSystem::OnInputCommand(const Events::InputCommand& e)
{
    return false;
}

void RenderSystem::Update(double dt)
{
    m_EventBroker->Process<RenderSystem>();

    // Update the current camera used for rendering
    if (m_CurrentCamera.Valid()) {
        m_Camera->SetPosition(Transform::AbsolutePosition(m_CurrentCamera));
        m_Camera->SetOrientation(Transform::AbsoluteOrientation(m_CurrentCamera));
    }

    RenderScene scene;
    scene.Camera = m_Camera;
    scene.Viewport = Rectangle(1280, 720);

    auto cSceneLight = m_World->GetComponents("SceneLight");
    if (cSceneLight != nullptr && cSceneLight->begin() != cSceneLight->end()) {
        m_RenderFrame->Gamma = (double)(*cSceneLight->begin())["Gamma"];
        m_RenderFrame->Exposure = (double)(*cSceneLight->begin())["Exposure"];
        scene.AmbientColor = (glm::vec4)(*cSceneLight->begin())["AmbientColor"];
    }

    fillModels(scene.OpaqueObjects, scene.TransparentObjects);
    fillPointLights(scene.PointLightJobs, m_World);
    fillDirectionalLights(scene.DirectionalLightJobs, m_World);
    fillText(scene.TextJobs, m_World);
    m_RenderFrame->Add(scene);
   
}