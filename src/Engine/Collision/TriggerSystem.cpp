#include "Collision/TriggerSystem.h"
#include "Collision/Collision.h"
#include "Core/AABB.h"
#include "Rendering/Model.h"

void TriggerSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    //Currently only players can trigger things.
    auto players = world->GetComponents("Player");
    if (players == nullptr) {
        return;
    }
    EntityID tId = component.EntityID;
    boost::optional<AABB> triggerBox = Collision::EntityAbsoluteAABB(entity);
    //The trigger *should* have a bounding box, or something, to test against so it can be triggered.
    if (!triggerBox) {
        return;
    }
    for (auto& pc : *players) {
        EntityID pId = pc.EntityID;
        boost::optional<AABB> playerBox = Collision::EntityAbsoluteAABB(EntityWrapper(world, pId));
        //The player can't trigger anything without an AABB.
        if (!playerBox) {
            continue;
        }
        if (!Collision::AABBVsAABB(*triggerBox, *playerBox)) {
            //Entity is not touching the trigger,
            //Throw event if it was previously.
            if (throwLeaveIfWasInTrigger(m_EntitiesTouchingTrigger[tId], pId, tId)) {
                continue;
            }
            //This only occurs if the entity was completely inside the trigger one frame, 
            //then completely outside the trigger, e.g. when dying and respawning.
            throwLeaveIfWasInTrigger(m_EntitiesCompletelyInTrigger[tId], pId, tId);
        } else {
            //Entity is at least touching the trigger.
            AABB completelyInsideBox = AABB::FromOriginSize((*triggerBox).Origin(), (*triggerBox).Size() - 2.0f * (*playerBox).Size());
            if (Collision::AABBVsAABB(completelyInsideBox, *playerBox) &&
                glm::all(glm::greaterThan((*triggerBox).Size(), (*playerBox).Size()))) {
                //Entity is completely inside the trigger.
                //If it was only touching before, it is erased.
                m_EntitiesTouchingTrigger[tId].erase(pId);
                std::unordered_set<EntityID>& completeSet = m_EntitiesCompletelyInTrigger[tId];
                if (completeSet.count(pId) == 0) {
                    //If it wasn't completely in the trigger, throw Enter and add to the set.
                    completeSet.insert(pId);
                    publish<Events::TriggerEnter>(pId, tId);
                }
            } else {
                //Entity is only touching the trigger.
                std::unordered_set<EntityID>& touchSet = m_EntitiesTouchingTrigger[tId];
                std::unordered_set<EntityID>& completeSet = m_EntitiesCompletelyInTrigger[tId];
                const auto& it = completeSet.find(pId);
                //If it was completely inside before.
                if (it != completeSet.end()) {
                    completeSet.erase(it);
                    touchSet.insert(pId);
                    //If it was completely outside before.
                } else if (touchSet.count(pId) == 0) {
                    publish<Events::TriggerTouch>(pId, tId);
                    touchSet.insert(pId);
                }
                //Else, it was touching the trigger last frame too and nothing is done.
            }
        }
    }
}

bool TriggerSystem::throwLeaveIfWasInTrigger(std::unordered_set<EntityID>& triggerSet, EntityID pId, EntityID tId)
{
    const auto& it = triggerSet.find(pId);
    if (it != triggerSet.end()) {
        //If it was in the trigger, but not anymore, throw leaveEvent and erase from the set.
        triggerSet.erase(it);
        publish<Events::TriggerLeave>(pId, tId);
        return true;
    }
    return false;
}

