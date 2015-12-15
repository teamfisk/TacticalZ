#ifndef TriggerSystem_h__
#define TriggerSystem_h__

#include <glm/common.hpp>
#include <unordered_set>

#include "Core/System.h"
#include "Core/EventBroker.h"
#include "ETrigger.h"

class AABB;

class TriggerSystem : public System
{
public:
    TriggerSystem(EventBroker* eventBroker)
        : System(eventBroker, "Trigger")
    {}

    virtual void Update(World* world, ComponentWrapper& collision, double dt) override;

private:
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_EntitiesTouchingTrigger;
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_EntitiesCompletelyInTrigger;

    bool getEntityBox(World* world, EntityID id, AABB& outBox);
    //True if leave event was thrown.
    bool throwLeaveIfWasInTrigger(std::unordered_set<EntityID>& triggerSet, EntityID pId, EntityID tId);
    void attachAABBComponentFromModel(World* world, EntityID id);
    template<typename Event>
    void publish(EntityID pId, EntityID tId)
    {
        Event e;
        e.Trigger = tId;
        e.Entity = pId;
        m_EventBroker->Publish(e);
    }
};

#endif