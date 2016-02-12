#include "Systems/HealthSystem.h"

HealthSystem::HealthSystem(SystemParams params)
    : System(params)
    , PureSystem("Health")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &HealthSystem::OnPlayerDamaged);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerHealthPickup, &HealthSystem::OnPlayerHealthPickup);
    EVENT_SUBSCRIBE_MEMBER(m_InputCommand, &HealthSystem::OnInputCommand);
}

void HealthSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cHealth, double dt)
{
    double& health = cHealth["Health"];
    if (health <= 0.0) {
        Events::PlayerDeath ePlayerDeath;
        ePlayerDeath.Player = entity;
        m_EventBroker->Publish(ePlayerDeath);
        //Note: we will delete the entity in PlayerDeathSystem
    }
}

bool HealthSystem::OnPlayerDamaged(Events::PlayerDamage& e)
{
    ComponentWrapper cHealth = e.Victim["Health"];
    double& health = cHealth["Health"];
    health -= e.Damage;

    return true;
}

bool HealthSystem::OnInputCommand(Events::InputCommand& e)
{
    if (e.Command == "TakeDamage" && e.Value > 0 && LocalPlayer.Valid()) {
        Events::PlayerDamage ev;
        ev.Victim = LocalPlayer;
        ev.Damage = e.Value;
        m_EventBroker->Publish(ev);
    }
    return true;
}

bool HealthSystem::OnPlayerHealthPickup(Events::PlayerHealthPickup& e)
{
    ComponentWrapper cHealth = e.Player["Health"];
    double& health = cHealth["Health"];
    health += e.HealthAmount;
    health = std::min(health, (double)cHealth["MaxHealth"]);

    return true;
}

