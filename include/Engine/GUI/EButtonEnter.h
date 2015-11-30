#ifndef Events_ButtonEnter_h__
#define Events_ButtonEnter_h__

#include "../Core/EventBroker.h"

namespace Events
{

/** Thrown on GUI button hover. */
struct ButtonEnter : Event
{
	std::string FrameName;
};

}

#endif