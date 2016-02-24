#ifndef EWin_h__
#define EWin_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{

//triggers when a team has captured all capturePoints
struct Win : Event
{
    // The winning team corresponding to TeamEnum
    ComponentInfo::EnumType Team;
};

}

#endif