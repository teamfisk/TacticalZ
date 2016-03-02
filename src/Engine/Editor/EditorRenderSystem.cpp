#include "Editor/EditorRenderSystem.h"

EditorRenderSystem::EditorRenderSystem(SystemParams params, IRenderer* renderer, RenderFrame* renderFrame)
    : System(params)
    , m_Renderer(renderer)
    , m_RenderFrame(renderFrame)
{
    EVENT_SUBSCRIBE_MEMBER(m_ESetCamera, &EditorRenderSystem::OnSetCamera);
    auto resolution = renderer->Resolution();
    m_Camera = new Camera((float)resolution.Width / resolution.Height, glm::radians(45.f), 0.01f, 5000.f);
}

void EditorRenderSystem::Update(double dt)
{
    if (m_CurrentCamera.Valid()) {
        m_Camera->SetPosition(Transform::AbsolutePosition(m_CurrentCamera));
        m_Camera->SetOrientation(Transform::AbsoluteOrientation(m_CurrentCamera));
    }

    RenderScene scene;
    scene.Camera = m_Camera;

    auto cameras = m_World->GetComponents("Camera");
    if (cameras != nullptr) {
        for (auto& cCamera : *cameras) {
            EntityWrapper entity(m_World, cCamera.EntityID);
            entity["Model"]["Resource"] = "Models/Widgets/Camera.mesh";
            glm::mat4 modelMatrix = Transform::ModelMatrix(entity);
            enqueueModel(scene, entity, modelMatrix, "Models/Widgets/Camera.mesh", entity["Model"], false);
        }
    }

    auto aabbs = m_World->GetComponents("AABB");
    if (aabbs != nullptr) {
        for (auto& cAABB : *aabbs) {
            EntityWrapper entity(m_World, cAABB.EntityID);
            glm::vec3 absPosition = Transform::AbsolutePosition(entity) + (glm::vec3)cAABB["Origin"];
            glm::vec3 absScale = Transform::AbsoluteScale(entity) * (glm::vec3)cAABB["Size"];
            glm::mat4 modelMatrix = glm::translate(absPosition) * glm::scale(absScale);
            entity["Model"]["Resource"] = "Models/Core/UnitCube.mesh";
            enqueueModel(scene, entity, modelMatrix, "Models/Core/UnitCube.mesh", entity["Model"], true);
        }
    }

    m_RenderFrame->Add(scene);
}

bool EditorRenderSystem::OnSetCamera(Events::SetCamera& e)
{
    ComponentWrapper cTransform = e.CameraEntity["Transform"];
    ComponentWrapper cCamera = e.CameraEntity["Camera"];
    m_Camera->SetFOV(static_cast<float>((double)cCamera["FOV"]));
    m_Camera->SetNearClip(static_cast<float>((double)cCamera["NearClip"]));
    m_Camera->SetFarClip(static_cast<float>((double)cCamera["FarClip"]));
    m_Camera->SetPosition(cTransform["Position"]);
    m_Camera->SetOrientation(glm::quat((const glm::vec3&)cTransform["Orientation"]));
    m_CurrentCamera = e.CameraEntity;
    return true;
}

void EditorRenderSystem::enqueueModel(RenderScene& scene, EntityWrapper entity, const glm::mat4& modelMatrix, const std::string& modelResource, ComponentWrapper cModel, bool wireframe)
{
    Model* model;
    try {
        model = ResourceManager::Load<::Model, true>(modelResource);
    } catch (const Resource::StillLoadingException&) {
        return;
    } catch (const std::exception&) {
        try {
            model = ResourceManager::Load<::Model>("Models/Core/Error.mesh");
        } catch (const std::exception&) {
            return;
        }
    }

    for (auto matGroup : model->MaterialGroups()) {
        std::shared_ptr<ModelJob> modelJob = std::shared_ptr<ModelJob>(new ModelJob(
                    model, 
                    m_Camera, 
                    modelMatrix, 
                    matGroup, 
                    cModel, 
                    m_World, 
                    glm::vec4(1.f, 1.f, 1.f, 1.f), 
                    0.f,
					false
                ));
        modelJob->Wireframe = wireframe;
        scene.Jobs.OpaqueObjects.push_back(modelJob);
    }
}

