#ifndef TriggerSystem_h__
#define TriggerSystem_h__

#include <glm/common.hpp>
#include <unordered_set>

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
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_EntitiesInTrigger;
};

#endif