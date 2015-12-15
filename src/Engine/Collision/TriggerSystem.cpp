#include "Collision/TriggerSystem.h"
#include "Core/AABB.h"

void TriggerSystem::Update(World* world, ComponentWrapper& trigger, double dt)
{
    auto players = world->GetComponents("Player");
    if (players == nullptr) {
        return;
    }
    //WTODO: Assumes box exists.
    ComponentWrapper& cBox = world->GetComponent(trigger.EntityID, "AABB");
    AABB aabb;
    aabb.CreateFromCenter(cBox["BoxCenter"], cBox["BoxSize"]);
    for (auto& c : *players) {

        Events::TriggerEnter e;
        e.Trigger = trigger.EntityID;
        e.Entity = 41;
        m_EventBroker->Publish(e);
    }
}

