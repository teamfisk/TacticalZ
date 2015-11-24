#ifndef Events_BindKey_h__
#define Events_BindKey_h__

#include "Core/EventBroker.h"

namespace Events
{

/** Called to bind a keyboard key to an input command. */
struct BindKey : Event
{
	/** The GLFW key code to bind. */
	int KeyCode;
	/** The command to send. */
	std::string Command;
	/** The value to send for positive stimulation.

		Multiplied by the 0-1 clamped value of the key.
	*/
	float Value;
};

}

#endif
