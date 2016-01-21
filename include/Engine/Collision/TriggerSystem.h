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
    TriggerSystem(EventBroker* eventBroker, Octree<AABB>* octree)
        : System(eventBroker)
        , PureSystem("Trigger")
        , m_Octree(octree)
    {
        EVENT_SUBSCRIBE_MEMBER(m_ETouch, &TriggerSystem::OnTouch);
        EVENT_SUBSCRIBE_MEMBER(m_EEnter, &TriggerSystem::OnEnter);
        EVENT_SUBSCRIBE_MEMBER(m_ELeave, &TriggerSystem::OnLeave);
    }

    virtual void UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    Octree<AABB>* m_Octree;
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_EntitiesTouchingTrigger;
    std::unordered_map<EntityID, std::unordered_set<EntityID>> m_EntitiesCompletelyInTrigger;

    //TODO: Only exists for debug purposes, remove later.
    EventRelay<TriggerSystem, Events::TriggerEnter> m_EEnter;
    bool OnEnter(const Events::TriggerEnter &event);
    EventRelay<TriggerSystem, Events::TriggerTouch> m_ETouch;
    bool OnTouch(const Events::TriggerTouch &event);
    EventRelay<TriggerSystem, Events::TriggerLeave> m_ELeave;
    bool OnLeave(const Events::TriggerLeave &event);

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