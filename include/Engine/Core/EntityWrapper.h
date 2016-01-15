#ifndef EntityWrapper_h__
#define EntityWrapper_h__

#include <boost/optional.hpp>
#include "ComponentWrapper.h"

class World;
struct EntityWrapper
{
    EntityWrapper()
        : World(nullptr)
        , ID(EntityID_Invalid)
    { }

    EntityWrapper(::World* world, EntityID id)
        : World(world)
        , ID(id)
    { }

    ::World* World;
    EntityID ID;

    bool HasComponent(const std::string& componentName);

    ComponentWrapper operator[](const std::string& componentName);
    bool operator==(const EntityWrapper& e);
    explicit operator EntityID();
};

#endif
