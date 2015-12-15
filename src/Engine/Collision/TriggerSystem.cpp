#include "Collision/TriggerSystem.h"
#include "Collision/Collision.h"
#include "Core/AABB.h"
#include "Rendering/Model.h"

void TriggerSystem::Update(World* world, ComponentWrapper& trigger, double dt)
{
    //Currently only players can trigger things.
    auto players = world->GetComponents("Player");
    if (players == nullptr) {
        return;
    }
    EntityID tId = trigger.EntityID;
    AABB triggerBox;
    //The trigger *should* have a bounding box, or something, to test against so it can be triggered.
    if (!getEntityBox(world, tId, triggerBox)) {
        return;
    }
    for (auto& pc : *players) {
        EntityID pId = pc.EntityID;
        AABB playerBox;
        //The player can't trigger anything without an AABB.
        if (!getEntityBox(world, pId, playerBox)) {
            continue;
        }
        if (!Collision::AABBVsAABB(triggerBox, playerBox)) {
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
            AABB completelyInsideBox;
            completelyInsideBox.CreateFromCenter(triggerBox.Center(), triggerBox.Size() - playerBox.Size());
            if (Collision::AABBVsAABB(completelyInsideBox, playerBox)) {
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

bool TriggerSystem::getEntityBox(World* world, EntityID id, AABB& outBox)
{
    //TODO: Improve checking if component exists. Remove try
    bool retry;
    do {
        retry = false;
        try {
            ComponentWrapper& cBox = world->GetComponent(id, "AABB");
            outBox.CreateFromCenter(cBox["BoxCenter"], cBox["BoxSize"]);
        } catch (std::out_of_range e) {
            retry = true;
            attachAABBComponentFromModel(world, id);
        }
    } while (retry);
    return true;
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

void TriggerSystem::attachAABBComponentFromModel(World* world, EntityID id)
{
    ComponentWrapper model = world->GetComponent(id, "Model");
    ComponentWrapper transform = world->GetComponent(id, "Transform");
    ComponentWrapper collision = world->AttachComponent(id, "AABB");
    Model* modelRes = ResourceManager::Load<Model>(model["Resource"]);
    
    glm::mat4 modelMatrix = modelRes->m_Matrix * 
        glm::translate(glm::mat4(), (glm::vec3)transform["Position"]) * 
        glm::toMat4((glm::quat)transform["Orientation"]) * 
        glm::scale((glm::vec3)transform["Scale"]);

    glm::vec3 mini = glm::vec3(INFINITY, INFINITY, INFINITY);
    glm::vec3 maxi = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
    for (const auto& v : modelRes->m_Vertices) {
        const auto& wPos = modelMatrix * glm::vec4(v.Position.x, v.Position.y, v.Position.z, 1);
        maxi.x = std::max(wPos.x, maxi.x);
        maxi.y = std::max(wPos.y, maxi.y);
        maxi.z = std::max(wPos.z, maxi.z);
        mini.x = std::min(wPos.x, mini.x);
        mini.y = std::min(wPos.y, mini.y);
        mini.z = std::min(wPos.z, mini.z);
    }
    collision["BoxCenter"] = 0.5f * (maxi + mini);
    collision["BoxSize"] = maxi - mini;
}
