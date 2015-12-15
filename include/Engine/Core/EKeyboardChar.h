#ifndef Events_KeyboardChar_h__
#define Events_KeyboardChar_h__

#include "EventBroker.h"

namespace Events
{

struct KeyboardChar : Event
{
    double Timestamp = 0.f;
	unsigned int Char = 0;
};

}

#endif