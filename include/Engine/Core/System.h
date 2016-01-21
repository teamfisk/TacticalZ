#ifndef System_h__
#define System_h__

#include "EventBroker.h"
#include "World.h"
#include "EntityWrapper.h"
#include "ComponentWrapper.h"

class System
{
    friend class SystemPipeline;

protected:
    System(World* world, EventBroker) { }
    System(World* world, EventBroker* eventBroker)
        : m_World(world)
        , m_EventBroker(eventBroker)
    { }
    virtual ~System() = default;

    World* m_World;
    EventBroker* m_EventBroker;
};

class PureSystem : public virtual System
{
    friend class SystemPipeline;

protected:
    PureSystem(std::string componentType)
        : m_ComponentType(componentType)
    { }
    virtual ~PureSystem() = default;

    const std::string m_ComponentType;

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cComponent, double dt) = 0;
};

class ImpureSystem : public virtual System
{
    friend class SystemPipeline;

protected:
    ImpureSystem() = default;
    virtual ~ImpureSystem() = default;

    virtual void Update(double dt) = 0;
};

#endif