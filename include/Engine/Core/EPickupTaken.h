#ifndef EPickupTaken_h__
#define EPickupTaken_h__

#include "Core/Event.h"
#include "Core/EntityWrapper.h"

namespace Events
{

struct PickupTaken : Event
{
    EntityID PickupID;
    EntityWrapper Spawner;
    EntityWrapper Player;
};

}

#endif