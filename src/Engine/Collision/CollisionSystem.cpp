#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"

void CollisionSystem::Update(World * world, ComponentWrapper & cAABB, double dt)
{
    //cAABB is any entity that should be collideable.
    m_EventBroker->Process<CollisionSystem>();
    //Currently the box is translated according to the models Transform matrix, 
    //but not scaled (or rotated since it's AABB). This should be fine, if we don't 
    //want to shrink or scale up an collideable object after creation.
    cAABB["BoxCenter"] = (glm::vec3)world->GetComponent(cAABB.EntityID, "Transform")["Position"];
    AABB thisBox;
    thisBox.CreateFromCenter(cAABB["BoxCenter"], cAABB["BoxSize"]);
    //Here c should be an object that moves, currently only players.
    if (zPress) {
        return;
    }
    for (auto& mover : *world->GetComponents("Player")) {
        if (cAABB.EntityID == mover.EntityID) {
            continue;
        }
        AABB otherBox;
        ComponentWrapper& aabbComp = world->GetComponent(mover.EntityID, "AABB");
        otherBox.CreateFromCenter(aabbComp["BoxCenter"], aabbComp["BoxSize"]);
        if (Collision::AABBVsAABB(thisBox, otherBox)) {
            ComponentWrapper& trans = world->GetComponent(mover.EntityID, "Transform");
            //TODO: Move entity to correct position on collision instead of this. Special treatment if both are movers.
            glm::vec3 newPos = trans["Position"];
            float moveSpeed = 0.12f;
            newPos += moveSpeed * glm::normalize(newPos - thisBox.Center());
            trans["Position"] = newPos;
            aabbComp["BoxCenter"] = newPos;
        }
    }
}

bool CollisionSystem::OnKeyUp(const Events::KeyUp & event)
{
    if (event.KeyCode == GLFW_KEY_Z) {
        zPress = true;
    } else if (event.KeyCode == GLFW_KEY_X) {
        zPress = false;
    }
    return false;
}
