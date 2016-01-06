#ifndef EPlayerDamage_h__
#define EPlayerDamage_h__

#include "EventBroker.h"
#include "../Core/Entity.h"

namespace Events
{

struct PlayerDamage : Event
{
    int DamageAmount;
    EntityID PlayerID;
};

}

#endif