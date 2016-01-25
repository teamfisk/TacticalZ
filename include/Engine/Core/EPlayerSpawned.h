#ifndef EPlayerSpawned_h__
#define EPlayerSpawned_h__

#include "Core/Event.h"
#include "Core/EntityWrapper.h"

namespace Events
{

struct PlayerSpawned : Event
{
    int PlayerID;
    EntityWrapper Player;
    EntityWrapper Spawner;
    std::string PlayerName;
};

}

#endif