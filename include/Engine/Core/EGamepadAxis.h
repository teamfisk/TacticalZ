#ifndef Events_GamepadAxis_h__
#define Events_GamepadAxis_h__

#include "EventBroker.h"

namespace Gamepad
{
	/** Gamepad axis type */
	enum class Axis
	{
		LeftX,
		LeftY,
		RightX,
		RightY,
		LeftTrigger,
		RightTrigger,
		LAST = RightTrigger
	};
}

namespace Events
{

/** Thrown when a gamepad axis changes value */
struct GamepadAxis : Event
{
	/** ID of the gamepad. */
	int GamepadID;
	/** The axis type. */
	Gamepad::Axis Axis;
	/** The new value of the axis. */
	float Value;
};

}

#endif