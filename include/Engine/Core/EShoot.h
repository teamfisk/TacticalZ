#ifndef EShoot_h__
#define EShoot_h__

#include "EventBroker.h"
#include "../Core/Entity.h"
#include "Engine/GLM.h"

namespace Events
{

struct Shoot : Event
{
    //ID for who made the shot
    EntityID shooter;
};

}

#endif