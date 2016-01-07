#ifndef EPlayerDamage_h__
#define EPlayerDamage_h__

#include "EventBroker.h"
#include "../Core/Entity.h"

namespace Events
{

struct PlayerDamage : Event
{
    double DamageAmount;
    EntityID PlayerDamagedID;
    //optional TypeOfDamage
    std::string TypeOfDamage;
};

}

#endif