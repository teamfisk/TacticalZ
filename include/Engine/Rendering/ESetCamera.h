#ifndef Events_SetCamera_h__
#define Events_SetCamera_h__

#include "../Core/EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct SetCamera : Event
{
    EntityWrapper CameraEntity;
};

}

#endif
