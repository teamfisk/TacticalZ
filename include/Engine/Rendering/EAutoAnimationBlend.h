#ifndef Events_AutoAnimationBlend_h__
#define Events_AutoAnimationBlend_h__

#include "../Core/EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

struct AutoAnimationBlend : Event
{
    EntityWrapper RootNode = EntityWrapper::Invalid;
    std::string NodeName;
    double Duration;

    EntityWrapper AnimationEntity = EntityWrapper::Invalid;
};

}

#endif
