#include "Collision/TriggerSystem.h"
#include "Collision/Collision.h"
#include "Core/AABB.h"
#include "Rendering/Model.h"

void TriggerSystem::UpdateComponent(EntityWrapper& triggerEntity, ComponentWrapper& cTrigger, double dt)
{
    boost::optional<EntityAABB> triggerBox = Collision::EntityAbsoluteAABB(triggerEntity);
    if (!triggerBox) {
        return;
    }

    Model* triggerModel = nullptr;
    glm::mat4 triggerModelMat;
    if (triggerEntity.HasComponent("Model")) {
        try {
            triggerModel = ResourceManager::Load<Model, true>(triggerEntity["Model"]["Resource"]);
            triggerModelMat = TransformSystem::ModelMatrix(triggerEntity);
        } catch (const std::exception&) {
        }
    }

    m_OctreeOut.clear();
    m_Octree->ObjectsInSameRegion(*triggerBox, m_OctreeOut);

    for (EntityAABB& colliderBox : m_OctreeOut) {
        EntityWrapper colliderEntity = colliderBox.Entity;

        if (Collision::AABBVsAABB(*triggerBox, colliderBox)) {
            AABB completelyInsideBox;
            bool colliderFitsInTrigger = glm::all(glm::greaterThan((*triggerBox).Size(), colliderBox.Size()));
            if (colliderFitsInTrigger) {
                completelyInsideBox = AABB::FromOriginSize((*triggerBox).Origin(), (*triggerBox).Size() - 2.0f * colliderBox.Size());
            }

            // We know the entity is inside the trigger box, but perhaps not the model yet.
            Collision::Output out = triggerModel == nullptr 
                ? Collision::Output::OutContained 
                : Collision::AABBvsTrianglesWContainment(
                colliderBox,
                triggerModel->m_Vertices,
                triggerModel->m_Indices,
                triggerModelMat);

            if (colliderFitsInTrigger && Collision::AABBVsAABB(completelyInsideBox, colliderBox) && out == Collision::Output::OutContained) {
                // Entity is completely inside the trigger.
                // If it was only touching before, it is erased.
                m_EntitiesTouchingTrigger[triggerEntity].erase(colliderEntity);
                auto& completeSet = m_EntitiesCompletelyInTrigger[triggerEntity];
                if (completeSet.count(colliderEntity) == 0) {
                    // If it wasn't completely in the trigger, throw Enter and add to the set.
                    completeSet.insert(colliderEntity);
                    publish<Events::TriggerEnter>(colliderEntity, triggerEntity);
                }
                continue;
            } else if (out != Collision::Output::OutSeparated) {
                // Entity is only touching the trigger.
                auto& touchSet = m_EntitiesTouchingTrigger[triggerEntity];
                auto& completeSet = m_EntitiesCompletelyInTrigger[triggerEntity];
                const auto& it = completeSet.find(colliderEntity);
                //If it was completely inside before.
                if (it != completeSet.end()) {
                    completeSet.erase(it);
                    touchSet.insert(colliderEntity);
                    //If it was completely outside before.
                } else if (touchSet.count(colliderEntity) == 0) {
                    publish<Events::TriggerTouch>(colliderEntity, triggerEntity);
                    touchSet.insert(colliderEntity);
                }
                // Else, it was touching the trigger last frame too and nothing is done.
                continue;
            }
        }
        // Only get here if entity is not touching the trigger,
        // throw event if it was touching previously.
        if (throwLeaveIfWasInTrigger(m_EntitiesTouchingTrigger[triggerEntity], colliderEntity, triggerEntity)) {
            continue;
        }
        // This only occurs if the entity was completely inside the trigger one frame, 
        // then completely outside the trigger, e.g. when dying and respawning.
        throwLeaveIfWasInTrigger(m_EntitiesCompletelyInTrigger[triggerEntity], colliderEntity, triggerEntity);
    }
}

bool TriggerSystem::throwLeaveIfWasInTrigger(std::unordered_set<EntityWrapper>& triggerSet, EntityWrapper colliderEntity, EntityWrapper triggerEntity)
{
    const auto& it = triggerSet.find(colliderEntity);
    if (it != triggerSet.end()) {
        //If it was in the trigger, but not anymore, throw leaveEvent and erase from the set.
        triggerSet.erase(it);
        publish<Events::TriggerLeave>(colliderEntity, triggerEntity);
        return true;
    }
    return false;
}

bool TriggerSystem::OnTouch(const Events::TriggerTouch &event)
{
    LOG_INFO("Player entity %i touched trigger entity %i.", event.Entity.ID, event.Trigger.ID);
    return true;
}

bool TriggerSystem::OnEnter(const Events::TriggerEnter &event)
{
    LOG_INFO("Player entity %i entered trigger entity %i.", event.Entity.ID, event.Trigger.ID);
    return true;
}

bool TriggerSystem::OnLeave(const Events::TriggerLeave &event)
{
    LOG_INFO("Player entity %i left trigger entity %i.", event.Entity.ID, event.Trigger.ID);
    return true;
}