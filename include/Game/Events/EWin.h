#ifndef EWin_h__
#define EWin_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{

//triggers when a team has captured all capturePoints
struct Win : Event
{
    //can be 0 = none, 1,2
    int TeamThatWon;
};

}

#endif