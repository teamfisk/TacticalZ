#ifndef EComponentDeleted_h__
#define EComponentDeleted_h__

#include "../Common.h"
#include "Event.h"
#include "Entity.h"

namespace Events
{

struct ComponentDeleted : Event
{
    EntityID Entity;
    std::string ComponentType;
    // True if the component was deleted as a result of the entity it was attached to being deleted
    bool Cascaded;
};

}

#endif