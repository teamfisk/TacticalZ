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
            //glm::vec3 gravity = glm::vec3(0, 9.82f * dt, 0);
            //TODO: glm::abs(inOutVelocity - gravity) doesn't work, because of velocity projection on ground normal.
            //if (!isOnGround || glm::any(glm::greaterThan(glm::abs(inOutVelocity - gravity), glm::vec3(0.0001f)))) {
                float verticalStepHeight = (float)(double)cPhysics["VerticalStepHeight"];
                if (Collision::AABBvsTriangles(boxA, model->m_Vertices, model->m_Indices, modelMatrix, inOutVelocity, verticalStepHeight, isOnGround, resolutionVector)) {
                    glm::vec3 pos = (glm::vec3)cTransform["Position"] + resolutionVector;
                    //Hack: Force the position to be at discrete values after collision, removes jittering.
                    //constexpr float discreteValue = 1.0e2f;
                    //pos = glm::vec3(glm::ivec3(discreteValue * pos)) / discreteValue;
                    //pos = glm::round(discreteValue * pos) / discreteValue;
                    (glm::vec3&)cTransform["Position"] = pos;
                    cPhysics["Velocity"] = inOutVelocity;
                    (bool)cPhysics["IsOnGround"] = isOnGround;
                } else {
                    (bool)cPhysics["IsOnGround"] = false;
                }
            //} else {
            //    (glm::vec3&)cTransform["Position"] -= gravity;
            //    cPhysics["Velocity"] = glm::vec3(0.f);
            //}
        } else if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
            //Enter here if boxB has no Model.
            (glm::vec3&)cTransform["Position"] += resolutionVector;
            (bool)cPhysics["IsOnGround"] = resolutionVector.y > 0;
            if ((bool)cPhysics["IsOnGround"]){
                ((glm::vec3&)cPhysics["Velocity"]).y = 0.f;
            }
        }
    }
}
