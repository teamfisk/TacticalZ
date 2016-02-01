#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"

void CollisionSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    if (!entity.HasComponent("Physics")) {
        return;
    }
    ComponentWrapper& cPhysics = entity["Physics"];

    boost::optional<EntityAABB> boundingBox = Collision::EntityAbsoluteAABB(entity);
    if (!boundingBox) {
        return;
    }
    ComponentWrapper& cTransform = entity["Transform"];
    EntityAABB& boxA = *boundingBox;

    // Collide against octree items
    m_OctreeResult.clear();
    m_Octree->ObjectsInSameRegion(*boundingBox, m_OctreeResult);
    for (auto& boxB : m_OctreeResult) {
        glm::vec3 resolutionVector;
        if (boxA.Entity == boxB.Entity) {
            continue;
        }

        if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
            (glm::vec3&)cTransform["Position"] += resolutionVector;
            if (resolutionVector.y > 0) {
                ((glm::vec3&)cPhysics["Velocity"]).y = 0.f;
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