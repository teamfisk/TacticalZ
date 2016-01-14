#ifndef Events_PauseSound_h__
#define Events_PauseSound_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{
// Pauses a playing sound
struct PauseSound : Event
{
    EntityID EmitterID;
};

}

#endif