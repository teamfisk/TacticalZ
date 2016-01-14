#ifndef TriggerSystem_h__
#define TriggerSystem_h__

#include <glm/common.hpp>
#include <unordered_set>

#include "../Core/System.h"
#include "../Core/EventBroker.h"
#include "../Core/Octree.h"
#include "ETrigger.h"

class AABB;

class TriggerSystem : public PureSystem
{
public:
    TriggerSystem(EventBroker* eventBroker, Octree* octree)
        : System(eventBroker)
        , PureSystem("Trigger")
        , m_Octree(octree)
    { }

    virtual void UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    Octree* m_Octree;
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_EntitiesTouchingTrigger;
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_EntitiesCompletelyInTrigger;

    //True if leave event was thrown.
    bool throwLeaveIfWasInTrigger(std::unordered_set<EntityID>& triggerSet, EntityID pId, EntityID tId);
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