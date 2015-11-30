#ifndef Events_ButtonRelease_h__
#define Events_ButtonRelease_h__

#include "../Core/EventBroker.h"

namespace GUI { class Button; }

namespace Events
{

/** Thrown on GUI button release. */
struct ButtonRelease : Event
{
	std::string FrameName;
	GUI::Button* Button;
};

}

#endif