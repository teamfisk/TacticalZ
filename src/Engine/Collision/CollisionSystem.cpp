#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"

void CollisionSystem::UpdateComponent(World * world, ComponentWrapper & cAABB, double dt)
{
    //Right now, cAABB is a component attached to any entity that should be collideable.
    AABB thisBox;
    if (!Collision::GetEntityBox(world, cAABB, thisBox)) {
        return;
    }
    //Press 'Z' to enable/disable collision.
    if (zPress) {
        return;
    }
    //Here, mover should be an object that moves, currently only players.
    for (auto& mover : *world->GetComponents("Player")) {
        if (cAABB.EntityID == mover.EntityID) {
            continue;
        }
        AABB otherBox;
        if (!Collision::GetEntityBox(world, mover.EntityID, otherBox)) {
            continue;
        }
        glm::vec3 resolveTranslation;
        if (Collision::AABBVsAABB(otherBox, thisBox, resolveTranslation)) {
            ComponentWrapper& trans = world->GetComponent(mover.EntityID, "Transform");
            //TODO: Special treatment if both are movers.
            trans["Position"] = (glm::vec3)trans["Position"] + resolveTranslation;
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
