#ifndef EEntityDeleted_h__
#define EEntityDeleted_h__

#include "Event.h"
#include "Entity.h"

namespace Events
{

struct EntityDeleted : Event
{
    EntityID DeletedEntity;
    // True if the entity deletion was triggered because the entity's parent was deleted before it
    bool Cascaded;
};

}

#endif