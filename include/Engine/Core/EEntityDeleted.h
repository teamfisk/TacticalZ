#ifndef EEntityDeleted_h__
#define EEntityDeleted_h__

#include "Event.h"
#include "EntityWrapper.h"

namespace Events
{

struct EntityDeleted : Event
{
    EntityWrapper DeletedEntity;
    // True if the entity deletion was triggered because the entity's parent was deleted before it
    bool Cascaded;
};

}

#endif