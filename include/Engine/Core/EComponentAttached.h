#ifndef EComponentAttached_h__
#define EComponentAttached_h__

#include "EventBroker.h"
#include "World.h"
#include "Entity.h"
#include "ComponentWrapper.h"

namespace Events
{

struct ComponentAttached : Event
{
    EntityWrapper Entity;
    ComponentWrapper Component;
};

}

#endif