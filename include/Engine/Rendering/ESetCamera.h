#ifndef Events_SetCamera_h__
#define Events_SetCamera_h__

#include "../Core/EventBroker.h"
#include "../Core/Entity.h"

namespace Events
{

/** Thrown Every frame, use functions to pick*/
struct SetCamera : Event
{
public:
    SetCamera() { };
    EntityID Entity;

private:

};

}

#endif
