#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"
#include "Rendering/Model.h"

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
    bool everHitTheGround = false;
    for (auto& boxB : m_OctreeResult) {
        glm::vec3 resolutionVector;
        if (boxA.Entity == boxB.Entity) {
            continue;
        }

        if (boxB.Entity.HasComponent("Model") && Collision::AABBVsAABB(boxA, boxB)) {
            //Here we know boxB is a entity with Collideable, AABB, and Model.
            RawModel* model;
            try {
                model = ResourceManager::Load<RawModel, true>(boxB.Entity["Model"]["Resource"]);
            } catch (const std::exception&) {
                continue;
            }

            glm::mat4 modelMatrix = Transform::ModelMatrix(boxB.Entity);

            glm::vec3 inOutVelocity = (glm::vec3)cPhysics["Velocity"];
            bool isOnGround = (bool)cPhysics["IsOnGround"];
            float verticalStepHeight = (float)(double)cPhysics["VerticalStepHeight"];
            if (Collision::AABBvsTriangles(boxA, model->Vertices(), model->m_Indices, modelMatrix, inOutVelocity, verticalStepHeight, isOnGround, resolutionVector)) {
                (glm::vec3&)cTransform["Position"] += resolutionVector;
                cPhysics["Velocity"] = inOutVelocity;
                if (isOnGround) {
                    everHitTheGround = true;
                    (bool)cPhysics["IsOnGround"] = true;
                }
            }
        } else if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
            //Enter here if boxB has no Model.
            (glm::vec3&)cTransform["Position"] += resolutionVector;
            if (resolutionVector.y > 0) {
                everHitTheGround = true;
                (bool)cPhysics["IsOnGround"] = true;
                ((glm::vec3&)cPhysics["Velocity"]).y = 0.f;
            }
        }
    }

    //This should apply air friction and such, iff zero models were hit.
    if (!everHitTheGround) {
        (bool)cPhysics["IsOnGround"] = false;
    }
}
