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
    
    void LockMouse();
    void UnlockMouse();
    virtual bool OnCommand(const Events::InputCommand& e) override;
    virtual void Reset();

protected:
    const int m_PlayerID;
    bool m_MouseLocked = false;
    glm::vec3 m_Rotation;
    glm::vec3 m_Movement;
    
    EventRelay<EventContext, Events::LockMouse> m_ELockMouse;
    bool OnLockMouse(const Events::LockMouse& e);
    EventRelay<EventContext, Events::UnlockMouse> m_EUnlockMouse;
    bool OnUnlockMouse(const Events::UnlockMouse& e);
};

#endif