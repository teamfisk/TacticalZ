#ifndef Events_ContinueSound_h__
#define Events_ContinueSound_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{
// Continues to play a sound from where it was paused.
struct ContinueSound : Event
{
    EntityID EmitterID;
};

}

#endif