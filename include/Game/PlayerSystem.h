#ifndef PlayerSystem_h__
#define PlayerSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Collision/ETrigger.h"
#include "Core\EMouseRelease.h"
#include "Core\EShoot.h"

class PlayerSystem : public PureSystem
{
public:
    PlayerSystem(EventBroker* eventBroker)
        : PureSystem(eventBroker, "Player")
    {
        EVENT_SUBSCRIBE_MEMBER(m_ETouch, &PlayerSystem::OnTouch);
        EVENT_SUBSCRIBE_MEMBER(m_EEnter, &PlayerSystem::OnEnter);
        EVENT_SUBSCRIBE_MEMBER(m_ELeave, &PlayerSystem::OnLeave);
        EVENT_SUBSCRIBE_MEMBER(m_MouseRelease, &PlayerSystem::OnMouseRelease);
    }

    virtual void UpdateComponent(World* world, ComponentWrapper& player, double dt) override;
private:
    float m_Speed = 5;
    bool leftMouseWasReleased = false;
    glm::vec2 aimingCoordinates;
    EventRelay<PlayerSystem, Events::TriggerEnter> m_EEnter;
    bool OnEnter(const Events::TriggerEnter &event);
    EventRelay<PlayerSystem, Events::TriggerTouch> m_ETouch;
    bool PlayerSystem::OnTouch(const Events::TriggerTouch &event);
    EventRelay<PlayerSystem, Events::TriggerLeave> m_ELeave;
    bool PlayerSystem::OnLeave(const Events::TriggerLeave &event);
    EventRelay<PlayerSystem, Events::MouseRelease> m_MouseRelease;
    bool PlayerSystem::OnMouseRelease(const Events::MouseRelease& e);
};

#endif