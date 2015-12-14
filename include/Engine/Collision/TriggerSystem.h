#ifndef PlayerSystem_h__
#define PlayerSystem_h__

#include <glm/common.hpp>

#include "Core/System.h"
#include "Core/EventBroker.h"
#include "ETrigger.h"

class TriggerSystem : public System
{
public:
    TriggerSystem(EventBroker* eventBroker)
        : System(eventBroker, "Trigger")
    {}

    virtual void Update(World* world, ComponentWrapper& collision, double dt) override;

private:
    
};

#endif