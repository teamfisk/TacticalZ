#ifndef Events_InputCommand_h__
#define Events_InputCommand_h__

#include "Core/EventBroker.h"
#include "Core/EntityWrapper.h"

namespace Events
{
/** Thrown when an input command is sent. */
struct InputCommand : Event
{
	/** Numerical ID of the player. */
	int PlayerID;
    EntityWrapper Player;
	/** The command that was sent. */
	std::string Command;
	/** The value of the command. */
	float Value = 0;
    /** Timestamp of the command. */
    double TimeStamp = 0;
};

}

#endif
