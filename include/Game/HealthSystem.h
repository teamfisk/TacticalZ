#ifndef HealthSystem_h__
#define HealthSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core\EPlayerDamage.h";
#include "Core\EPlayerHealthPickup.h";
#include "Core\EPlayerDeath.h";

class HealthSystem : public PureSystem
{
public:
    HealthSystem(EventBroker* eventBroker);

    virtual void UpdateComponent(World* world, ComponentWrapper& player, double dt) override;
private:
    float m_Speed = 5;

    //create the methods which will take care of specific events
    EventRelay<HealthSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool HealthSystem::OnPlayerDamaged(const Events::PlayerDamage& e);
    EventRelay<HealthSystem, Events::PlayerHealthPickup> m_EPlayerHealthPickup;
    bool HealthSystem::OnPlayerHealthPickup(const Events::PlayerHealthPickup& e);

    int playerDeltaHealth;

};

#endif