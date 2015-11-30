#ifndef Events_GamepadButton_h__
#define Events_GamepadButton_h__

#include "EventBroker.h"

namespace Gamepad
{
	/** Gamepad button type */
	enum class Button
	{
		Up,
		Down,
		Left,
		Right,
		Start,
		Back,
		LeftThumb,
		RightThumb,
		LeftShoulder,
		RightShoulder,
		A,
		B,
		X,
		Y,
		LAST = Y
	};
}

namespace Events
{

/** Thrown on gamepad button press. */
struct GamepadButtonDown : Event
{
	/** ID of the gamepad. */
	int GamepadID;
	/** The button that was pressed. */
	Gamepad::Button Button;
};

/** Thrown on gamepad button release. */
struct GamepadButtonUp : Event
{
	/** ID of the gamepad. */
	int GamepadID;
	/** The button that was released. */
	Gamepad::Button Button;
};

}

#endif
