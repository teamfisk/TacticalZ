#ifndef EPlayerDeath_h__
#define EPlayerDeath_h__

#include "EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct PlayerDeath : Event
{
    //KilledBy,KilledByWhat is optional for now. It might be used later in the playerlog-system
    EntityID KilledBy;
    EntityID PlayerID;
    EntityWrapper Player;
    std::string KilledByWhat;
};

}

#endif