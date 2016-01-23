#include "Input/FirstPersonInputController.h"

template <typename EventContext>
FirstPersonInputController<EventContext>::FirstPersonInputController(EventBroker* eventBroker, int playerID) 
    : InputController(eventBroker)
    , m_PlayerID(playerID)
{
    EVENT_SUBSCRIBE_MEMBER(m_ELockMouse, &FirstPersonInputController::OnLockMouse);
    EVENT_SUBSCRIBE_MEMBER(m_EUnlockMouse, &FirstPersonInputController::OnUnlockMouse);
}

template <typename EventContext>
void FirstPersonInputController<EventContext>::Reset()
{
    m_Rotation = glm::vec3(0.f, 0.f, 0.f);
}

template <typename EventContext>
void FirstPersonInputController<EventContext>::LockMouse()
{
    Events::LockMouse e;
    m_EventBroker->Publish(e);
    m_MouseLocked = true;
}

template <typename EventContext>
void FirstPersonInputController<EventContext>::UnlockMouse()
{
    Events::UnlockMouse e;
    m_EventBroker->Publish(e);
    m_MouseLocked = false;
}

template <typename EventContext>
bool FirstPersonInputController<EventContext>::OnCommand(const Events::InputCommand& e)
{
    if (m_PlayerID != e.PlayerID) {
        return false;
    }

    if (e.Command == "Pitch") {
        float val = glm::radians(e.Value);
        m_Rotation.x += -val;
        m_Rotation.x = glm::clamp(m_Rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());
        //m_Rotation = m_Rotation * glm::angleAxis<float>(-val, glm::vec3(1.f, 0, 0));
        return true;
    }

    if (e.Command == "Yaw") {
        float val = glm::radians(e.Value);
        m_Rotation.y += -val;
        //m_Rotation = glm::angleAxis<float>(-val, glm::vec3(0, 1.f, 0)) * m_Rotation;
        return true;
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

template <typename EventContext>
bool FirstPersonInputController<EventContext>::OnUnlockMouse(const Events::UnlockMouse& e)
{
    m_MouseLocked = false; 
    return true;
}

template <typename EventContext>
bool FirstPersonInputController<EventContext>::OnLockMouse(const Events::LockMouse& e)
{
    m_MouseLocked = true; 
    return true;
}
