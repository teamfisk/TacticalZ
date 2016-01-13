#ifndef EShoot_h__
#define EShoot_h__

#include "EventBroker.h"
#include "../Core/Entity.h"
#include "Engine/GLM.h"

namespace Events
{

struct Shoot : Event
{
    //shotgun etc has different amounts of damage probably (a sniper shot might one-shot)
    //also different weapons will have different spread
    int CurrentlyEquippedItem;
    //currentAimingPoint must be sent, in case the camera is moved while the event is being processed
    glm::vec2 CurrentAimingPoint;
};

}

#endif