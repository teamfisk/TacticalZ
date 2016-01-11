#ifndef Events_StopSound_h__
#define Events_StopSound_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{

struct StopSound : Event
{
    EntityID EmitterID;
};

}

#endif