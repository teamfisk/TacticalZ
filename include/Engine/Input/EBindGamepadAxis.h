#ifndef Events_BindGamepadAxis_h__
#define Events_BindGamepadAxis_h__

#include "Core/EventBroker.h"
#include "Core/EGamepadAxis.h"

namespace Events
{

/** Called to bind a gamepad axis to an input command. */
struct BindGamepadAxis : Event
{
	/** The axis to bind. */
	Gamepad::Axis Axis;
	/** The command to send. */
	std::string Command;
	/** The value to send for positive stimulation.
	
		Multiplied by the 0-1 clamped value of the axis.
	*/
	float Value;
};

}
