#ifndef EShoot_h__
#define EShoot_h__

#include "EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct Shoot : Event
{
    EntityWrapper Player;
};

}

#endif