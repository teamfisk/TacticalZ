#ifndef Events_ButtonReleased_h__
#define Events_ButtonReleased_h__

#include "Core/Event.h"

namespace Events
{

struct ButtonReleased : public Event {
    EntityWrapper Entity;
};

}

#endif