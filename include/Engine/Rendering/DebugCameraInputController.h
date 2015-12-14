#include "../Input/FirstPersonInputController.h"

template <typename EventContext>
class DebugCameraInputController : public FirstPersonInputController<EventContext>
{
public:
    DebugCameraInputController(EventBroker* eventBroker, unsigned int playerID)
        : FirstPersonInputController(eventBroker, playerID)
    { }

    const glm::vec3 Position() const { return m_Position; }
    void SetBaseSpeed(float speed) { m_BaseSpeed = speed; }

    virtual bool OnCommand(const Events::InputCommand& e) override
    {
        if (e.Command == "PrimaryFire") {
            if (e.Value > 0) {
                LockMouse();
            } else {
                UnlockMouse();
            }
            return false;
        }

        if (e.Command == "Right") {
            float value = std::max(-1.f, std::min(e.Value, 1.f));
            m_Velocity.x = value;
        }
        if (e.Command == "Forward") {
            float value = std::max(-1.f, std::min(e.Value, 1.f));
            m_Velocity.z = -value;
        }

        return FirstPersonInputController::OnCommand(e);
    }

    void Update(double dt)
    {
        if (glm::length2(m_Velocity) > 0) {
            m_Position += m_Orientation * (glm::normalize(m_Velocity) * m_BaseSpeed);
        }
    }

protected:
    glm::vec3 m_Position = glm::vec3(0, 0, 0);
    glm::vec3 m_Velocity = glm::vec3(0, 0, 0);
    float m_BaseSpeed = 0.1f;
};