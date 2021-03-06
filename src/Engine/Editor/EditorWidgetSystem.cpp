#include "Editor/EditorWidgetSystem.h"

EditorWidgetSystem::EditorWidgetSystem(SystemParams params, IRenderer* renderer) 
    : System(params)
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
    
    Events::WidgetDelta e;

    // Widget axes should have a common parent
    EntityWrapper widgetBase = entity.Parent();
    if (!widgetBase.Valid()) {
        widgetBase = entity;
    }
    glm::vec3 widgetBasePos = widgetBase["Transform"]["Position"];
    glm::quat widgetBaseOri = glm::quat((glm::vec3)widgetBase["Transform"]["Orientation"]);

    auto camera = m_PickData.Camera;
    glm::vec3 axis = (glm::vec3)cEditorWidget["Axis"];
    glm::vec3 axisOriented = widgetBaseOri * axis;
    glm::vec2 axisScreen = camera->WorldToScreen(widgetBasePos + axisOriented, m_Renderer->GetViewportSize()) - camera->WorldToScreen(widgetBasePos, m_Renderer->GetViewportSize());
    float dot = glm::dot(m_MouseDelta, glm::normalize(axisScreen)) / glm::length(axisScreen);
    glm::vec3 worldMovement = dot * axis;

    ComponentWrapper::SubscriptProxy& type = cEditorWidget["Type"];
    if ((ComponentInfo::EnumType)type == type.Enum("Translate")) {
        e.Translation += worldMovement;
        m_EventBroker->Publish(e);
    } else if ((ComponentInfo::EnumType)type == type.Enum("Rotate")) {
        //if (glm::length(worldMovement) > 0) {
        //    glm::vec3& orientation = moveEntity["Transform"]["Orientation"];
        //    glm::quat q = glm::quat(orientation);
        //    q *= glm::quat(glm::vec3(worldMovement));
        //    orientation = glm::eulerAngles(q);
        //}
    }

    m_MouseDelta = glm::vec2(0);
}

bool EditorWidgetSystem::OnMouseMove(const Events::MouseMove& e)
{
    m_MouseDelta = glm::vec2((float)e.DeltaX, (float)-e.DeltaY);
    return false;
}

bool EditorWidgetSystem::OnMousePress(const Events::MousePress & e)
{
    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureMouse && !io.WantCaptureKeyboard && e.Button == GLFW_MOUSE_BUTTON_1) {
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