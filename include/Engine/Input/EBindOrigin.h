#ifndef Events_BindOrigin_h__
#define Events_BindOrigin_h__

#include "Core/EventBroker.h"

namespace Events
{

/** Called to bind an input origin to an input command. */
struct BindOrigin : Event
{
	/** The input origin to bind. */
	std::string Origin;
	/** The command to send. */
	std::string Command;
	/** The value to send for positive stimulation. */
	float Value = 1.f;
};

}

#endif
