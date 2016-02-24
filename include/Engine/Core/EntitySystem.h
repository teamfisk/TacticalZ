#ifndef EntitySystem_h__
#define EntitySystem_h__

#include "World.h"
#include "SystemPipeline.h"

// An "EntitySystem" is defined as the combination of a World with a SystemPipeline
template <typename WorldType>
class EntitySystem : public WorldType, public SystemPipeline
{
    static_assert(std::is_base_of<World, WorldType>::value, "WorldType must inherit from World");
public:
    EntitySystem(EventBroker* eventBroker, bool isClient, bool isServer)
        : WorldType(eventBroker)
        , SystemPipeline(this, eventBroker, isClient, isServer)
    { }
};

#endif