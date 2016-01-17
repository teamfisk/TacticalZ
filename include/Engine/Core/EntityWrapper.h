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

    static const EntityWrapper Invalid;

    bool HasComponent(const std::string& componentName);
    bool Valid();

    ComponentWrapper operator[](const char* componentName);
    bool operator==(const EntityWrapper& e);
    explicit operator EntityID() const;
    operator bool();
};

#endif
