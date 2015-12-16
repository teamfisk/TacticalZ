#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"

void CollisionSystem::UpdateComponent(World * world, ComponentWrapper & cAABB, double dt)
{
    //cAABB is any entity that should be collideable.
    AABB thisBox;
    if (!Collision::GetEntityBox(world, cAABB, thisBox)) {
        return;
    }
    //Here c should be an object that moves, currently only players.
    if (zPress) {
        return;
    }
    for (auto& mover : *world->GetComponents("Player")) {
        if (cAABB.EntityID == mover.EntityID) {
            continue;
        }
        AABB otherBox;
        if (!Collision::GetEntityBox(world, mover.EntityID, otherBox)) {
            continue;
        }
        if (Collision::AABBVsAABB(thisBox, otherBox)) {
            ComponentWrapper& trans = world->GetComponent(mover.EntityID, "Transform");
            //TODO: Move entity to correct position on collision instead of this. Special treatment if both are movers.
            glm::vec3 newPos = trans["Position"];
            float moveSpeed = 0.12f;
            newPos += moveSpeed * glm::normalize(newPos - thisBox.Center());
            trans["Position"] = newPos;
        }
    }
}

bool CollisionSystem::OnKeyUp(const Events::KeyUp & event)
{
    if (event.KeyCode == GLFW_KEY_Z) {
        zPress = !zPress;
    }
    return false;
}
