#ifndef Events_StopSound_h__
#define Events_StopSound_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{
// Stops a sound emitter, and will also delete it.
struct StopSound : Event
{
    EntityID EmitterID;
};

}

#endif