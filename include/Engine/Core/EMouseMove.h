#ifndef Events_MouseMove_h__
#define Events_MouseMove_h__

#include "Core/EventBroker.h"

namespace Events
{

/** Thrown on mouse movement */
struct MouseMove : Event
{
	/** The new position of the cursor in window coordinates. */
	double X, Y;
	/** The delta movement of the mouse. */
	double DeltaX, DeltaY;
};

}

#endif
