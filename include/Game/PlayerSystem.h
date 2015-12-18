#ifndef PlayerSystem_h__
#define PlayerSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core/EventBroker.h"
#include "Core/EKeyDown.h"
#include "Core/EKeyUp.h"
#include "Collision/ETrigger.h"

struct KeyInput
{
    bool Forward = false;
    bool Left = false;
    bool Back = false;
    bool Right = false;
};

class PlayerSystem : public PureSystem
{
public:
    PlayerSystem(EventBroker* eventBroker)
        : PureSystem(eventBroker, "Player")
    {
        EVENT_SUBSCRIBE_MEMBER(m_EKeyDown, &PlayerSystem::OnKeyDown);
        EVENT_SUBSCRIBE_MEMBER(m_EKeyUp, &PlayerSystem::OnKeyUp);
        EVENT_SUBSCRIBE_MEMBER(m_ETouch, &PlayerSystem::OnTouch);
        EVENT_SUBSCRIBE_MEMBER(m_EEnter, &PlayerSystem::OnEnter);
        EVENT_SUBSCRIBE_MEMBER(m_ELeave, &PlayerSystem::OnLeave);
    }

    virtual void UpdateComponent(World* world, ComponentWrapper& player, double dt) override;

private:
    float m_Speed = 5;
    glm::vec3 m_Direction;
    KeyInput input;

    EventRelay<PlayerSystem, Events::KeyDown> m_EKeyDown;
    bool OnKeyDown(const Events::KeyDown &event);
    EventRelay<PlayerSystem, Events::KeyUp> m_EKeyUp;
    bool OnKeyUp(const Events::KeyUp &event);
    EventRelay<PlayerSystem, Events::TriggerEnter> m_EEnter;
    bool OnEnter(const Events::TriggerEnter &event);
    EventRelay<PlayerSystem, Events::TriggerTouch> m_ETouch;
    bool PlayerSystem::OnTouch(const Events::TriggerTouch &event);
    EventRelay<PlayerSystem, Events::TriggerLeave> m_ELeave;
    bool PlayerSystem::OnLeave(const Events::TriggerLeave &event);
};

#endif