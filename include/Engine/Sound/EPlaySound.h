#ifndef Events_PlaySound_h__
#define Events_PlaySound_h__

#include "Core/EventBroker.h"
#include "Core/Entity.h"

namespace Events
{

    struct PlaySound : Event
{
    std::string FilePath;
    EntityID emitter;
};

}

#endif