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
    double Duration = 0.0;
    double Delay = 0.0;


    double AnimationSpeed = 1.0;
    bool Restart = false;

    EntityWrapper AnimationEntity = EntityWrapper::Invalid;
};

}

#endif
