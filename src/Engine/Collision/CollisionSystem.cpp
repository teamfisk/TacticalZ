#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"

void CollisionSystem::UpdateComponent(World* world, ComponentWrapper& cAABB, double dt)
{
    EntityID entity = cAABB.EntityID;
    boost::optional<AABB> boundingBox = Collision::EntityAbsoluteAABB(world, entity);
    if (!boundingBox) {
        return;
    }
    ComponentWrapper& cTransform = world->GetComponent(entity, "Transform");
    AABB& boxA = *boundingBox;

    //Press 'Z' to enable/disable collision.
    if (zPress) {
        return;
    }

    std::vector<AABB> octreeResult;
    m_Octree->BoxesInSameRegion(*boundingBox, octreeResult);
    for (auto& boxB : octreeResult) {
        glm::vec3 resolutionVector;
        if (Collision::IsSameBoxProbably(boxA, boxB)) {
            continue;
        }
        if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
            (glm::vec3&)cTransform["Position"] += resolutionVector;
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
