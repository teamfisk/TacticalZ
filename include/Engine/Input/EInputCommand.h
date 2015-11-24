#ifndef Events_InputCommand_h__
#define Events_InputCommand_h__

#include "Core/EventBroker.h"

namespace Events
{
/** Thrown when an input command is sent. */
struct InputCommand : Event
{
	/** Numerical ID of the player. */
	unsigned int PlayerID;
	/** The command that was sent. */
	std::string Command;
	/** The value of the command. */
	float Value;
};

}

#endif
