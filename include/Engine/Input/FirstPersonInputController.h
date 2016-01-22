#ifndef FirstPersonInputController_h__
#define FirstPersonInputController_h__

#include "../GLM.h"
#include "../Core/InputController.h"
#include "../Core/ELockMouse.h"

template <typename EventContext>
class FirstPersonInputController : public InputController<EventContext>
{
public:
    FirstPersonInputController(EventBroker* eventBroker, int playerID)
        : InputController(eventBroker)
        , m_PlayerID(playerID)
    {
        EVENT_SUBSCRIBE_MEMBER(m_ELockMouse, &FirstPersonInputController::OnLockMouse);
        EVENT_SUBSCRIBE_MEMBER(m_EUnlockMouse, &FirstPersonInputController::OnUnlockMouse);
    }

    virtual const glm::vec3 Movement() const { return m_Movement; }
    virtual const glm::vec3 Orientation() const { return m_Orientation; }
    
    void LockMouse()
    {
        Events::LockMouse e;
        m_EventBroker->Publish(e);
        m_MouseLocked = true;
    }

    void UnlockMouse()
    {
        Events::UnlockMouse e;
        m_EventBroker->Publish(e);
        m_MouseLocked = false;
    }

    virtual bool OnCommand(const Events::InputCommand& e) override
    {
        if (m_PlayerID != e.PlayerID) {
            return false;
        }

        if (m_MouseLocked) {
            if (e.Command == "Pitch") {
                float val = glm::radians(e.Value);
                m_Orientation.x += -val;
                m_Orientation.x = glm::clamp(m_Orientation.x, -glm::half_pi<float>(), glm::half_pi<float>());
                //m_Orientation = m_Orientation * glm::angleAxis<float>(-val, glm::vec3(1.f, 0, 0));
                return true;
            }

            if (e.Command == "Yaw") {
                float val = glm::radians(e.Value);
                m_Orientation.y += -val;
                //m_Orientation = glm::angleAxis<float>(-val, glm::vec3(0, 1.f, 0)) * m_Orientation;
                return true;
            }
        }

        if (e.Command == "Forward" || e.Command == "Right") {
            if (e.Command == "Forward") {
                float val = glm::clamp(e.Value, -1.f, 1.f);
                m_Movement.z = -val;
                return true;
            }
            if (e.Command == "Right") {
                float val = glm::clamp(e.Value, -1.f, 1.f);
                m_Movement.x = val;
                return true;
            }
            if (glm::length2(m_Movement) > 0) {
                m_Movement = glm::normalize(m_Movement);
            }
        }

        return false;
    }

protected:
    const int m_PlayerID;
    bool m_MouseLocked = false;
    glm::vec3 m_Orientation;
    glm::vec3 m_Movement;
    
    EventRelay<EventContext, Events::LockMouse> m_ELockMouse;
    bool OnLockMouse(const Events::LockMouse& e) { m_MouseLocked = true; return true; }
    EventRelay<EventContext, Events::UnlockMouse> m_EUnlockMouse;
    bool OnUnlockMouse(const Events::UnlockMouse& e) { m_MouseLocked = false; return true; }
};

#endif