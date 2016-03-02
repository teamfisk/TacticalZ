#ifndef ECaptured_h__
#define ECaptured_h__

#include "EventBroker.h"
#include "../Core/Entity.h"
#include "Engine/GLM.h"

namespace Events
{

    //triggers when a capturePoint has been taken over
struct Captured : Event
{
    int TeamNumberThatCapturedCapturePoint;
    EntityID CapturePointTakenID;
    EntityWrapper NextCapturePoint;
};

}

#endif