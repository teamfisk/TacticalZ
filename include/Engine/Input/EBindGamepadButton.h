#ifndef Events_BindGamepadButton_h__
#define Events_BindGamepadButton_h__

#include "Core/EventBroker.h"
#include "Core/EGamepadButton.h"

namespace Events
{

/** Called to bind a gamepad button to an input command. */
struct BindGamepadButton : Event
{
	/** The gamepad button to bind. */
	Gamepad::Button Button;
	/** The command to send. */
	std::string Command;
	/** The value to send for positive stimulation.

		Multiplied by the 0-1 clamped value of the button.
	*/
	float Value;
};

}

#endif
