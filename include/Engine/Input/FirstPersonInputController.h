#ifndef FirstPersonInputController_h__
#define FirstPersonInputController_h__

#include "../GLM.h"
#include "../Core/InputController.h"
#include "../Core/ELockMouse.h"

template <typename EventContext>
class FirstPersonInputController : public InputController<EventContext>
{
public:
    FirstPersonInputController(EventBroker* eventBroker, unsigned int playerID)
        : InputController(eventBroker)
        , m_PlayerID(playerID)
    {
        EVENT_SUBSCRIBE_MEMBER(m_ELockMouse, &FirstPersonInputController::OnLockMouse);
        EVENT_SUBSCRIBE_MEMBER(m_EUnlockMouse, &FirstPersonInputController::OnUnlockMouse);
    }

    const glm::quat Orientation() const { return m_Orientation; }
    
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
                m_Orientation = m_Orientation * glm::angleAxis<float>(-val, glm::vec3(1, 0, 0));
                return true;
            }

            if (e.Command == "Yaw") {
                float val = glm::radians(e.Value);
                m_Orientation = glm::angleAxis<float>(-val, glm::vec3(0, 1, 0)) * m_Orientation;
                return true;
            }
        }

        return false;
    }

protected:
    const unsigned int m_PlayerID;
    glm::quat m_Orientation;
    bool m_MouseLocked = false;
    
    EventRelay<EventContext, Events::LockMouse> m_ELockMouse;
    bool OnLockMouse(const Events::LockMouse& e) { m_MouseLocked = true; return true; }
    EventRelay<EventContext, Events::UnlockMouse> m_EUnlockMouse;
    bool OnUnlockMouse(const Events::UnlockMouse& e) { m_MouseLocked = false; return true; }
};

#endif