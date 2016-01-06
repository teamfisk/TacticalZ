#ifndef EPlayerHealthPickup_h__
#define EPlayerHealthPickup_h__

#include "EventBroker.h"
#include "../Core/Entity.h"

namespace Events
{

struct PlayerHealthPickup : Event
{
    double HealthAmount;
    EntityID HealthPickupID;
    EntityID playerHealedID;
};

}

#endif