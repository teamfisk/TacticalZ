#include "HealthSystem.h"

HealthSystem::HealthSystem(EventBroker* eventBroker)
    : PureSystem(eventBroker, "Health")
{
    //subscribe/listenTo playerdamage,healthpickup events with the eventbroker
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerDamage, &HealthSystem::OnPlayerDamaged);
    EVENT_SUBSCRIBE_MEMBER(m_EPlayerHealthPickup, &HealthSystem::OnPlayerHealthPickup);
    playerDeltaHealth = 0;
}
void HealthSystem::UpdateComponent(World * world, ComponentWrapper & player, double dt)
{
    //Health is only affected by pickup/shoot events
    player["Health"] += playerDeltaHealth;
    playerDeltaHealth = 0;
    if (player["Health"] < 0) {
        //sendout/publish death event
        Events::PlayerDeath e;
        e.PlayerID = player.EntityID;
        m_EventBroker->Publish(e);
    }
}
bool HealthSystem::OnPlayerDamaged(const Events::PlayerDamage& e)
{
    //vem skadades?
    //antagligen spelaren själv
    //såna här events skickas av network te spelare som kan lyssna på / kolla på de och tar hand om sina egna +-hp endast
    //just add that damage to a variable, which will later be taken care of by UpdateComponent
    playerDeltaHealth -= e.DamageAmount;
    //ev skicka ut playerdeath event
    return true;
}
bool HealthSystem::OnPlayerHealthPickup(const Events::PlayerHealthPickup& e)
{
    //vem tog upp hp?
    playerDeltaHealth += e.HealthAmount;
    return true;
}
