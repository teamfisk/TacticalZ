#include "HealthSystem.h"
#include <algorithm>

HealthSystem::HealthSystem(EventBroker* eventBroker)
    : PureSystem(eventBroker, "Health")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &HealthSystem::OnPlayerDamaged);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerHealthPickup, &HealthSystem::OnPlayerHealthPickup);
}

void HealthSystem::UpdateComponent(World *world, ComponentWrapper &health, double dt)
{
    //if entityID of health is 9 then the players ID is also 9 (player,health are connected to the same entity)
    ComponentWrapper player = world->GetComponent(health.EntityID, "Player");
    double currentHealth;
    double maxHealth = (double)world->GetComponent(health.EntityID, "Health")["MaxHealth"];

    //process the DeltaHealthVector and change the entitys health accordingly
    for (size_t i = m_DeltaHealthVector.size(); i > 0; i--)
    {
        auto deltaHP = m_DeltaHealthVector[i - 1];
        //if we have a healthchange for the current player, then apply it
        if (std::get<0>(deltaHP) == player.EntityID) {
            //re-read currentHealth for each iteration
            currentHealth = (double)world->GetComponent(health.EntityID, "Health")["Health"];
            //get the deltaHP value from the tuple and make sure you dont get more than maxHealth
            double newHealth = std::min(currentHealth + (double)std::get<1>(deltaHP), maxHealth);
            health.SetProperty("Health", newHealth);
            m_DeltaHealthVector.erase(m_DeltaHealthVector.begin() + i - 1);
        }
    }

    currentHealth = (double)world->GetComponent(health.EntityID, "Health")["Health"];
    //check if health is <= 0
    if (currentHealth <= 0.0f) {
        //publish death event
        Events::PlayerDeath e;
        e.PlayerID = player.EntityID;
        m_EventBroker->Publish(e);
    }
}

bool HealthSystem::OnPlayerDamaged(const Events::PlayerDamage& e)
{
    //save the changed HP to a vector. it will be taken care of in UpdateComponent
    m_DeltaHealthVector.push_back(std::make_tuple(e.PlayerDamagedID, -e.DamageAmount));
    return true;
}

bool HealthSystem::OnPlayerHealthPickup(const Events::PlayerHealthPickup& e)
{
    //save the changed HP to a vector. it will be taken care of in UpdateComponent
    m_DeltaHealthVector.push_back(std::make_tuple(e.PlayerHealedID, e.HealthAmount));
    return true;
}
