#ifndef Events_KeyDown_h__
#define Events_KeyDown_h__

#include "EventBroker.h"

namespace Events
{

/** Thrown on key press. */
struct KeyDown : Event
{
	/** GLFW key code */
	int KeyCode;
    bool ModCtrl;
    bool ModAlt;
    bool ModShift;
};

}

#endif
