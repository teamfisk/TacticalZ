#ifndef FirstPersonInputController_h__
#define FirstPersonInputController_h__

#include "../GLM.h"
#include "../Core/InputController.h"
#include "../Core/ELockMouse.h"

template <typename EventContext>
class FirstPersonInputController : public InputController<EventContext>
{
public:
    FirstPersonInputController(EventBroker* eventBroker, int playerID);

    virtual const glm::vec3 Movement() const { return m_Movement; }
    virtual const glm::vec3 Rotation() const { return m_Rotation; }
    virtual bool Jumping() const { return m_Jumping; }
    virtual bool Crouching() const { return m_Crouching; }
    virtual bool DoubleJumping() const { return m_DoubleJumping; }
    virtual void SetDoubleJumping(bool isDoubleJumping) {
        m_DoubleJumping = isDoubleJumping;
    }

    void LockMouse();
    void UnlockMouse();
    virtual bool OnCommand(const Events::InputCommand& e) override;
    virtual void Reset();

protected:
    const int m_PlayerID;
    bool m_MouseLocked = false;
    glm::vec3 m_Rotation;
    glm::vec3 m_Movement;
    bool m_Jumping = false;
    bool m_DoubleJumping = false;
    bool m_Crouching = false;

    EventRelay<EventContext, Events::LockMouse> m_ELockMouse;
    bool OnLockMouse(const Events::LockMouse& e);
    EventRelay<EventContext, Events::UnlockMouse> m_EUnlockMouse;
    bool OnUnlockMouse(const Events::UnlockMouse& e);
};

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
    m_Jumping = false;
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
        //m_Rotation.x = glm::clamp(m_Rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());
    }

    if (e.Command == "Yaw") {
        float val = glm::radians(e.Value);
        m_Rotation.y += -val;
    }

    if (e.Command == "Forward" || e.Command == "Right") {
        if (e.Command == "Forward") {
            float val = glm::clamp(e.Value, -1.f, 1.f);
            m_Movement.z = -val;
        }
        if (e.Command == "Right") {
            float val = glm::clamp(e.Value, -1.f, 1.f);
            m_Movement.x = val;
        }
        if (glm::length2(m_Movement) > 0) {
            m_Movement = glm::normalize(m_Movement);
        }
    }

    if (e.Command == "Jump") {
        m_Jumping = e.Value > 0;
    }

    if (e.Command == "Crouch") {
        m_Crouching = e.Value > 0;
    }

    return true;
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

#endif