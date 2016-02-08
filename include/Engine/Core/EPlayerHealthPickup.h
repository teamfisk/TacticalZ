#ifndef EPlayerHealthPickup_h__
#define EPlayerHealthPickup_h__

#include "EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

    struct PlayerHealthPickup : Event
    {
        EntityWrapper Player;
        double HealthAmount;
    };

}

#endif