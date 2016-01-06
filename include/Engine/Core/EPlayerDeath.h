#ifndef EPlayerDeath_h__
#define EPlayerDeath_h__

#include "EventBroker.h"
#include "../Core/Entity.h"

namespace Events
{

struct PlayerDeath : Event
{
    std::string KilledBy;
    EntityID PlayerID;
};

}

#endif