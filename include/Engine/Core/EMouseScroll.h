#ifndef Events_MouseScroll_h__
#define Events_MouseScroll_h__

#include "EventBroker.h"

namespace Events
{

struct MouseScroll : Event
{
    double DeltaX;
    double DeltaY;
};

}

#endif
