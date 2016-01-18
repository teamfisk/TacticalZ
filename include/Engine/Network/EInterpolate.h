#ifndef Events_Interpolate_h__
#define Events_Interpolate_h__

#include <boost/shared_array.hpp>

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{

struct Interpolate : Event
{
    EntityID Entity;
    boost::shared_array<char*> DataArray;
};

}

#endif