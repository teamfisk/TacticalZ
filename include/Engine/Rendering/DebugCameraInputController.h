#include <imgui/imgui.h>
#include "../Input/FirstPersonInputController.h"

template <typename EventContext>
class DebugCameraInputController : public FirstPersonInputController<EventContext>
{
public:
    DebugCameraInputController(EventBroker* eventBroker, unsigned int playerID)
        : FirstPersonInputController(eventBroker, playerID)
    { }

    void SetPosition(const glm::vec3 position) { m_Position = position; }
    void SetOrientation(const glm::quat orientation) { m_Orientation = orientation; }

    const glm::vec3 Position() const { return m_Position; }
    void SetBaseSpeed(float speed) { m_BaseSpeed = speed; }

    virtual bool OnCommand(const Events::InputCommand& e) override
    {
        ImGuiIO& io = ImGui::GetIO();

        if (e.Command == "PrimaryFire") {
            if (e.Value > 0) {
                if (!io.WantCaptureMouse) {
                    LockMouse();
                }
            } else {
                UnlockMouse();
            }
            return false;
        }

        if (!io.WantCaptureKeyboard) {
            if (e.Command == "Right") {
                float value = std::max(-1.f, std::min(e.Value, 1.f));
                m_Velocity.x = value;
            }
            if (e.Command == "Forward") {
                float value = std::max(-1.f, std::min(e.Value, 1.f));
                m_Velocity.z = -value;
            }
            if (e.Command == "Sprint") {
                if (e.Value > 0.f) {
                    m_Speed = m_BaseSpeed * 2.f * (e.Value);
                } else {
                    m_Speed = m_BaseSpeed;
                }
            }
        }

        return FirstPersonInputController::OnCommand(e);
    }

    void Update(double dt)
    {
        if (glm::length2(m_Velocity) > 0) {
            m_Position += m_Orientation * (glm::normalize(m_Velocity) * m_Speed * (float)dt);
        }
    }

protected:
    glm::vec3 m_Position = glm::vec3(0, 0, 0);
    glm::vec3 m_Velocity = glm::vec3(0, 0, 0);
    float m_BaseSpeed = 50.0f;//2.0f;
    float m_Speed = m_BaseSpeed;
};