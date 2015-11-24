#ifndef Events_MouseRelease_h__
#define Events_MouseRelease_h__

#include "Core/EventBroker.h"

namespace Events
{

/** Thrown on mouse button release. */
struct MouseRelease : Event
{
	/** GLFW mouse button code */
	int Button;
	/** The mouse position in window coordinates. */
	double X, Y;
};

}

#endif
