#ifndef Events_BindMouseButton_h__
#define Events_BindMouseButton_h__

#include "Core/EventBroker.h"

namespace Events
{

/** Called to bind a mouse button to an input command. */
struct BindMouseButton : Event
{
	/** The GLFW mouse button code to bind. */
	int Button;
	/** The command to send. */
	std::string Command;
	/** The value to send for positive stimulation.

		Multiplied by the 0-1 clamped value of the button.
	*/
	float Value;
};

}

#endif
