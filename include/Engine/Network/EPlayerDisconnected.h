#ifndef Events_PlayerDisconnected
#define Events_PlayerDisconnected

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{

struct PlayerDisconnected : public Event
{
    unsigned int PlayerID;
    EntityID Entity;
};

}

#endif
