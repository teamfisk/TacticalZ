#include "Systems/HealthSystem.h"

HealthSystem::HealthSystem(EventBroker* eventBroker)
    : System(eventBroker)
    , PureSystem("Health")
{
    //subscribe/listenTo playerdamage,healthpickup events (using the eventBroker)
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &HealthSystem::OnPlayerDamaged);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerHealthPickup, &HealthSystem::OnPlayerHealthPickup);
}

void HealthSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    //if entityID of health is 9 then the players ID is also 9 (player,health are connected to the same entity)
    ComponentWrapper player = world->GetComponent(component.EntityID, "Player");
    double maxHealth = (double)component["MaxHealth"];

    //process the DeltaHealthVector and change the entitys health accordingly
    for (size_t i = m_DeltaHealthVector.size(); i > 0; i--)
    {
        auto deltaHP = m_DeltaHealthVector[i - 1];
        //if we have a healthchange for the current player and health is greater than 0, then apply it
        if (std::get<0>(deltaHP) == player.EntityID && (double)component["Health"] > 0.0f) {
            //get the deltaHP value from the tuple and make sure you dont get more than maxHealth
            double newHealth = std::min((double)component["Health"] + (double)std::get<1>(deltaHP), maxHealth);
            component["Health"] = newHealth;
            m_DeltaHealthVector.erase(m_DeltaHealthVector.begin() + i - 1);
            //check if health is <= 0
            if ((double)component["Health"] <= 0.0f) {
                health["Health"] = 0.0;
                //publish death event
                Events::PlayerDeath e;
                e.PlayerID = player.EntityID;
                m_EventBroker->Publish(e);
                //clear the remaining hpDeltas for the dead player
                for (size_t j = m_DeltaHealthVector.size(); j > 0; j--)
                {
                    if (std::get<0>(m_DeltaHealthVector[j - 1]) == player.EntityID)
                        m_DeltaHealthVector.erase(m_DeltaHealthVector.begin() + j - 1);
                }
                //break the loop if the player is dead
                break;
            }
        }
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
