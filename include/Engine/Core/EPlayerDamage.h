#ifndef EPlayerDamage_h__
#define EPlayerDamage_h__

#include "EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct PlayerDamage : Event
{
    EntityWrapper Player;
    double Damage;
};

}

#endif