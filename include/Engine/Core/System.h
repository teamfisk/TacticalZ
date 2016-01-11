#ifndef System_h__
#define System_h__

#include "EventBroker.h"
#include "World.h"
#include "ComponentWrapper.h"

class System
{
    friend class SystemPipeline;

protected:
    System(EventBroker* eventBroker)
        : m_EventBroker(eventBroker)
    { }
    virtual ~System() = default;

    EventBroker* m_EventBroker;
};

class PureSystem : public System
{
    friend class SystemPipeline;

protected:
    PureSystem(EventBroker* eventBroker, std::string componentType)
        : System(eventBroker)
        , m_ComponentType(componentType)
    { }
    virtual ~PureSystem() = default;

    const std::string m_ComponentType;

    virtual void UpdateComponent(World* world, ComponentWrapper& component, double dt) = 0;
};

class ImpureSystem : public System
{
    friend class SystemPipeline;

protected:
    ImpureSystem(EventBroker* eventBroker)
        : System(eventBroker)
    { }
    virtual ~ImpureSystem() = default;

    virtual void Update(World* world, double dt) = 0;
};

#endif