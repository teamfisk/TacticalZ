#ifndef EPickupSpawned_h__
#define EPickupSpawned_h__

#include "Core/Event.h"
#include "Core/EntityWrapper.h"

namespace Events
{

struct PickupSpawned : Event
{
    EntityWrapper Pickup;
};

}

#endif