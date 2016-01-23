#include "Editor/EditorRenderSystem.h"

EditorRenderSystem::EditorRenderSystem(World* m_World, EventBroker* eventBroker, IRenderer* renderer, RenderFrame* renderFrame) 
    : System(m_World, eventBroker)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
{
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &EditorRenderSystem::OnSetCamera);
    auto resolution = Rectangle::Rectangle(1280, 720);
    m_EditorCamera = new Camera((float)resolution.Width / resolution.Height, glm::radians(45.f), 0.01f, 5000.f);
}

void EditorRenderSystem::Update(double dt)
{
    if (m_CurrentCamera) {
        ComponentWrapper cameraTransform = m_CurrentCamera["Transform"];
        m_EditorCamera->SetPosition(cameraTransform["Position"]);
        m_EditorCamera->SetOrientation(glm::quat((const glm::vec3&)cameraTransform["Orientation"]));
    }

    RenderScene scene;
    scene.ClearDepth = true;
    scene.Camera = m_EditorCamera;
    scene.Viewport = Rectangle(1920, 1080);

    auto models = m_World->GetComponents("Model");
    if (models != nullptr) {
        for (auto& cModel : *models) {
            if (!(bool)cModel["Visible"]) {
                continue;
            }

            const std::string& resource = cModel["Resource"];

            Model* model;
            try {
                model = ResourceManager::Load<::Model, true>(resource);
            } catch (const Resource::StillLoadingException&) {
                continue;
            } catch (const std::exception&) {
                try {
                    model = ResourceManager::Load<::Model>("Models/Core/Error.obj");
                } catch (const std::exception&) {
                    continue;
                }
            }

            EntityWrapper entity(m_World, cModel.EntityID);
            glm::mat4 modelMatrix = Transform::ModelMatrix(entity.ID, entity.World);
            for (auto matGroup : model->MaterialGroups()) {
                std::shared_ptr<ModelJob> modelJob = std::make_shared<ModelJob>(model, scene.Camera, modelMatrix, matGroup, cModel, entity.World, glm::vec4(0), 0.f);
                scene.ForwardJobs.push_back(modelJob);
            }
        }
    }

    auto pointLights = m_World->GetComponents("PointLight");
    if (pointLights != nullptr) {
        for (auto& cPointLight : *pointLights) {
            bool visible = cPointLight["Visible"];
            if (!visible) {
                continue;
            }

            EntityWrapper entity(m_World, cPointLight.EntityID);
            ComponentWrapper& cTransform = entity["Transform"];
            std::shared_ptr<PointLightJob> pointLightJob = std::make_shared<PointLightJob>(cTransform, cPointLight, entity.World);
            scene.PointLightJobs.push_back(pointLightJob);
        }
    }

    m_RenderFrame->Add(scene);
}

bool EditorRenderSystem::OnSetCamera(Events::SetCamera& e)
{
    ComponentWrapper cTransform = e.CameraEntity["Transform"];
    ComponentWrapper cCamera = e.CameraEntity["Camera"];
    m_EditorCamera->SetFOV((double)cCamera["FOV"]);
    m_EditorCamera->SetNearClip((double)cCamera["NearClip"]);
    m_EditorCamera->SetFarClip((double)cCamera["FarClip"]);
    m_EditorCamera->SetPosition(cTransform["Position"]);
    m_EditorCamera->SetOrientation(glm::quat((const glm::vec3&)cTransform["Orientation"]));
    m_CurrentCamera = e.CameraEntity;
    return true;
}

