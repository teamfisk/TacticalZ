#ifndef Events_MousePress_h__
#define Events_MousePress_h__

#include "EventBroker.h"

namespace Events
{

/** Thrown on mouse button press. */
struct MousePress : Event
{
	/** GLFW mouse button code */
	int Button;
	/** The click position in window coordinates. */
	double X, Y;
};

}

#endif
