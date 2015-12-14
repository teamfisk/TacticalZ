#ifndef Events_TriggerEnter_h__
#define Events_TriggerEnter_h__

#include "../Core/EventBroker.h"
#include "../Core/Entity.h"

namespace Events
{

/** Thrown once, when an entity is completely inside a trigger. */
struct TriggerTouch : Event
{
    /** The id of the entity that touches the trigger. */
    EntityID Entity;
    /** The id of the trigger entity. */
    EntityID Trigger;
};

/** Thrown once, when an entity has completely left a trigger. */
struct TriggerLeave : Event
{
    /** The id of the entity that left the trigger. */
    EntityID Entity;
    /** The id of the trigger entity. */
    EntityID Trigger;
};

/** Thrown when an entity is completely inside a trigger. */
struct TriggerEnter : Event
{
    /** The id of the entity that entered the trigger. */
    EntityID Entity;
    /** The id of the trigger entity. */
    EntityID Trigger;
};

}

#endif
