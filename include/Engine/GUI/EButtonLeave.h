#ifndef Events_ButtonLeave_h__
#define Events_ButtonLeave_h__

#include "../Core/EventBroker.h"

namespace Events
{

/** Thrown on GUI button hover. */
struct ButtonLeave : Event
{
	std::string FrameName;
};

}

#endif