#ifndef EPlayerDeath_h__
#define EPlayerDeath_h__

#include "EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct PlayerDeath : Event
{
    //KilledBy,KilledByWhat is optional for now. It might be used later in the playerlog-system
    EntityWrapper Player = EntityWrapper::Invalid;
    EntityWrapper Killer = EntityWrapper::Invalid;
    std::string KilledByWhat = "";
};

}

#endif