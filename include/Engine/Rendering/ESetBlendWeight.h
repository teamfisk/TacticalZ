#ifndef Events_SetBlendWeight_h__
#define Events_SetBlendWeight_h__

#include "../Core/EventBroker.h"
#include "../Core/EntityWrapper.h"

namespace Events
{

//Sets the blend weight for all nodes with "NodeName"
struct SetBlendWeight : Event
{
    EntityWrapper RootNode = EntityWrapper::Invalid;
    std::string NodeName;
    double Weight;
};

}

#endif
