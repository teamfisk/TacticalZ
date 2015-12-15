#ifndef Events_CreatePlayer_h__
#define Events_CreatePlayer_h__

#include "Core/EventBroker.h"
#include "Core/World.h"

namespace Events
{

struct CreatePlayer : Event
{
    unsigned int entityID;
    std::string modelPath;
    World* world;
};

}

#endif
