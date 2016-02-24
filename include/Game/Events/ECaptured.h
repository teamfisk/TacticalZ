#ifndef ECaptured_h__
#define ECaptured_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{

//triggers when a capturePoint has been taken over
struct Captured : Event
{
    int TeamNumberThatCapturedCapturePoint;
    EntityID CapturePointID;
};

}

#endif