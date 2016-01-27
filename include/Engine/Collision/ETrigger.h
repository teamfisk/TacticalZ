#ifndef Events_TriggerEnter_h__
#define Events_TriggerEnter_h__

#include "../Core/EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

/** Thrown once, when an entity is only touching a trigger. */
struct TriggerTouch : Event
{
    /** The id of the entity that touches the trigger. */
    EntityWrapper Entity;
    /** The id of the trigger entity. */
    EntityWrapper Trigger;
};

/** Thrown once, when an entity has completely left a trigger. */
struct TriggerLeave : Event
{
    /** The id of the entity that left the trigger. */
    EntityWrapper Entity;
    /** The id of the trigger entity. */
    EntityWrapper Trigger;
};

/** Thrown once, when an entity is completely contained inside a trigger. */
struct TriggerEnter : Event
{
    /** The id of the entity that entered the trigger. */
    EntityWrapper Entity;
    /** The id of the trigger entity. */
    EntityWrapper Trigger;
};

}

#endif
