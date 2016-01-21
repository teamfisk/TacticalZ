#ifndef EPause_h__
#define EPause_h__

#include "EventBroker.h"
#include "World.h"

namespace Events
{

struct Pause : Event 
{
    ::World* World;
};

struct Resume : Event 
{
    ::World* World;
};

}

#endif