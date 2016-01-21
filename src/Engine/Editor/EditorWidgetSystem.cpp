#include "Editor/EditorWidgetSystem.h"

EditorWidgetSystem::EditorWidgetSystem(World* world, EventBroker* eventBroker, IRenderer* renderer) 
    : System(world, eventBroker)
    , PureSystem("EditorWidget")
    , m_Renderer(renderer)
{
    EVENT_SUBSCRIBE_MEMBER(m_EMouseMove, &EditorWidgetSystem::OnMouseMove);
    EVENT_SUBSCRIBE_MEMBER(m_EMousePress, &EditorWidgetSystem::OnMousePress);
    EVENT_SUBSCRIBE_MEMBER(m_EMouseRelease, &EditorWidgetSystem::OnMouseRelease);
}

void EditorWidgetSystem::Update(double dt)
{
    // Pick at current mouse position
}

void EditorWidgetSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cEditorWidget, double dt)
{
    if (!m_PickEntity.Valid() || m_PickEntity != entity) {
        return;
    }
    
    EntityWrapper moveEntity = entity.Parent();
    if (!moveEntity.Valid()) {
        moveEntity = entity;
    }

    ComponentWrapper::SubscriptProxy& type = cEditorWidget["Type"];
    if ((ComponentInfo::EnumType)type == type.Enum("Translate")) {
        auto camera = m_PickData.Camera;
        glm::vec3 axis = cEditorWidget["Axis"];
        glm::vec2 axisScreen = camera->WorldToScreen(axis, m_Renderer->Resolution()) - camera->WorldToScreen(glm::vec3(0, 0, 0), m_Renderer->Resolution());
        float dot = glm::dot(m_MouseDelta, glm::normalize(axisScreen)) / glm::length(axisScreen);
        glm::vec3 worldMovement = dot * axis;
        (glm::vec3&)moveEntity["Transform"]["Position"] += worldMovement;
    }

    m_MouseDelta = glm::vec2(0);
}

void EditorWidgetSystem::debugPrintVector(const char* name, glm::vec2 axisNDC)
{
    ImGui::Text("%s: (%f, %f)", name, axisNDC.x, axisNDC.y);
}
void EditorWidgetSystem::debugPrintVector(const char* name, glm::vec4 axisNDC)
{
    ImGui::Text("%s: (%f, %f, %f, %f)", name, axisNDC.x, axisNDC.y, axisNDC.z, axisNDC.w);
}
void EditorWidgetSystem::debugPrintVector(const char* name, glm::vec3 axisNDC)
{
    ImGui::Text("%s: (%f, %f, %f)", name, axisNDC.x, axisNDC.y, axisNDC.z);
}

bool EditorWidgetSystem::OnMouseMove(const Events::MouseMove& e)
{
    m_MouseDelta = glm::vec2((float)e.DeltaX, (float)-e.DeltaY);
    return false;
}

bool EditorWidgetSystem::OnMousePress(const Events::MousePress & e)
{
    if (e.Button == GLFW_MOUSE_BUTTON_2) {
        m_PickData = m_Renderer->Pick(glm::vec2(e.X, e.Y));
        if (m_PickData.Entity != EntityID_Invalid && m_PickData.World == m_World) {
            m_PickEntity = EntityWrapper(m_World, m_PickData.Entity);
        }
    }
    return true;
}

bool EditorWidgetSystem::OnMouseRelease(const Events::MouseRelease& e)
{
    m_PickEntity = EntityWrapper::Invalid;
    return true;
}