#ifndef Events_ButtonPress_h__
#define Events_ButtonPress_h__

#include "../Core/EventBroker.h"

namespace GUI { class Button; }

namespace Events
{

/** Thrown on GUI button press. */
struct ButtonPress : Event
{
	std::string FrameName;
	GUI::Button* Button;
};

}

#endif