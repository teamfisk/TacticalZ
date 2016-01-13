#ifndef System_h__
#define System_h__

#include "EventBroker.h"
#include "World.h"
#include "ComponentWrapper.h"

class System
{
    friend class SystemPipeline;

protected:
    System()
        : m_EventBroker(nullptr)
    { }
    System(EventBroker* eventBroker)
        : m_EventBroker(eventBroker)
    { }
    virtual ~System() = default;

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

    virtual void UpdateComponent(World* world, ComponentWrapper& component, double dt) = 0;
};

class ImpureSystem : public virtual System
{
    friend class SystemPipeline;

protected:
    ImpureSystem() = default;
    virtual ~ImpureSystem() = default;

    virtual void Update(World* world, double dt) = 0;
};

#endif