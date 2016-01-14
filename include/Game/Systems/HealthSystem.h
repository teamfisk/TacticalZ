#ifndef HealthSystem_h__
#define HealthSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "Common.h"
#include "Core/System.h"
#include "Core/EPlayerDamage.h"
#include "Core/EPlayerHealthPickup.h"
#include "Core/EPlayerDeath.h"

#include <tuple>
#include <vector>

class HealthSystem : public PureSystem
{
public:
    HealthSystem(EventBroker* eventBroker);

    //updatecomponent
    virtual void UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    //methods which will take care of specific events
    EventRelay<HealthSystem, Events::PlayerDamage> m_EPlayerDamage;
    bool HealthSystem::OnPlayerDamaged(const Events::PlayerDamage& e);
    EventRelay<HealthSystem, Events::PlayerHealthPickup> m_EPlayerHealthPickup;
    bool HealthSystem::OnPlayerHealthPickup(const Events::PlayerHealthPickup& e);

    //vector which will keep track of health changes
    std::vector<std::tuple<EntityID, double>> m_DeltaHealthVector;
   
};

#endif