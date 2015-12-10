#ifndef System_h__
#define System_h__

#include "EventBroker.h"
#include "World.h"
#include "ComponentWrapper.h"

class System
{
    friend class SystemPipeline;

public:
    System(const EventBroker* eventBroker, std::string componentType)
        : m_EventBroker(eventBroker)
        , m_ComponentType(componentType)
    { }

    virtual void Update(World* world, ComponentWrapper& component, double dt) = 0;

private:
    const EventBroker* m_EventBroker;
    std::string m_ComponentType;
};

#endif