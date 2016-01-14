#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"

void CollisionSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    if (!entity.HasComponent("Physics")) {
        return;
    }
    ComponentWrapper& cPhysics = entity["Physics"];

    boost::optional<AABB> boundingBox = Collision::EntityAbsoluteAABB(entity);
    if (!boundingBox) {
        return;
    }
    ComponentWrapper& cTransform = entity["Transform"];
    AABB& boxA = *boundingBox;

    //Press 'Z' to enable/disable collision.
    if (zPress) {
        return;
    }

    // Collide against octree
    std::vector<AABB> octreeResult;
    m_Octree->BoxesInSameRegion(*boundingBox, octreeResult);
    for (auto& boxB : octreeResult) {
        glm::vec3 resolutionVector;
        if (Collision::IsSameBoxProbably(boxA, boxB)) {
            continue;
        }
        if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
            if (entity.HasComponent("Physics")) {
                (glm::vec3&)cTransform["Position"] += resolutionVector;
                cPhysics["Velocity"] = glm::vec3(0, 0, 0);
            }
        }
    }

    // HACK: Temporarily collide against all collidable models since they're not in the octree yet
    //auto otherCollidables = world->GetComponents("Model");
    //for (auto& cModel : *otherCollidables) {
    //    if (cModel.EntityID == entity) {
    //        continue;
    //    }
    //    if (!world->HasComponent(cModel.EntityID, "Collidable")) {
    //        continue;
    //    }

    //    auto absPosition = RenderQueueFactory::AbsolutePosition(world, cModel.EntityID);
    //    auto absOrientation = RenderQueueFactory::AbsoluteOrientation(world, cModel.EntityID);
    //    auto absScale = RenderQueueFactory::AbsoluteScale(world, cModel.EntityID);
    //    glm::mat4 modelMatrix = glm::translate(absPosition); // *glm::toMat4(absOrientation) * glm::scale(absScale);

    //    auto model = ResourceManager::Load<Model>(cModel["Resource"]);
    //    glm::vec3 resolutionVector;
    //    if (Collision::AABBvsTriangles(boxA, model->m_Vertices, model->m_Indices, modelMatrix, resolutionVector)) {
    //        (glm::vec3&)cTransform["Position"] += resolutionVector;
    //    }
    //}
}

bool CollisionSystem::OnKeyUp(const Events::KeyUp & event)
{
    if (event.KeyCode == GLFW_KEY_Z) {
        zPress = !zPress;
    }
    return false;
}
