#ifndef Events_KillDeath_h__
#define Events_KillDeath_h__

#include "Core/EventBroker.h"

typedef unsigned int PlayerID;

namespace Events
{

struct KillDeath : public Event 
{
    PlayerID Casualty = -1;
    PlayerID Killer = -1;
};

}

#endif