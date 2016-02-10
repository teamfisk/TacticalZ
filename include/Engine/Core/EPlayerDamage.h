#ifndef EPlayerDamage_h__
#define EPlayerDamage_h__

#include "EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct PlayerDamage : Event
{
    //NOTE: this struct is missing information on what the damageSource is
    EntityWrapper Player;
    double Damage;
};

}

#endif