#ifndef Events_AnimationComplete_h__
#define Events_AnimationComplete_h__

#include "../Core/EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct AnimationComplete : Event
{
    EntityWrapper Entity;
    std::string Name;
};

}

#endif
