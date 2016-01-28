#include "Systems/HealthSystem.h"

HealthSystem::HealthSystem(World* m_World, EventBroker* eventBroker)
    : System(m_World, eventBroker)
    , PureSystem("Health")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &HealthSystem::OnPlayerDamaged);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerHealthPickup, &HealthSystem::OnPlayerHealthPickup);
}

void HealthSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
}

bool HealthSystem::OnPlayerDamaged(Events::PlayerDamage& e)
{
    ComponentWrapper cHealth = e.Player["Health"];
    double& health = cHealth["Health"];
    health -= e.Damage;
    
    if (health <= 0.0) {
        Events::PlayerDeath ePlayerDeath;
        ePlayerDeath.PlayerID = e.Player.ID;
        ePlayerDeath.Player = e.Player;
        m_EventBroker->Publish(ePlayerDeath);
        //Note: we will delete the entity in PlayerDeathSystem
    }

    return true;
}

bool HealthSystem::OnPlayerHealthPickup(Events::PlayerHealthPickup& e)
{
    ComponentWrapper cHealth = e.Player["Health"];
    double& health = cHealth["Health"];
    //NOTE: its possible to get more than MaxHealth health
    health += e.HealthAmount;

    return true;
}

