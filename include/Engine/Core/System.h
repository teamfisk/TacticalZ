#ifndef System_h__
#define System_h__

#include "EventBroker.h"
#include "World.h"
#include "ComponentWrapper.h"

class System
{
    friend class SystemPipeline;

public:
    System(EventBroker* eventBroker, std::string componentType)
        : m_EventBroker(eventBroker)
        , m_ComponentType(componentType)
    { }
	
	virtual void Initialize();
    virtual void Update(World* world, ComponentWrapper& component, double dt) = 0;

private:
    std::string m_ComponentType;
	EventBroker* m_EventBroker;
};

#endif