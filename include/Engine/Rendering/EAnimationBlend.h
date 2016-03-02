#ifndef Events_AnimationBlend_h__
#define Events_AnimationBlend_h__

#include "../Core/EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct AnimationBlend : Event
{
    EntityWrapper BlendEntity = EntityWrapper::Invalid;
    double GoalWeight;
    double Duration;

    EntityWrapper AnimationEntity = EntityWrapper::Invalid;
};

}

#endif
