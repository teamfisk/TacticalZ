#include "Collision/Collision.h"
#include "Collision/CollisionSystem.h"
#include "Core/AABB.h"
#include "Rendering/Model.h"

void CollisionSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& cPhysics, double dt)
{
    if (!entity.HasComponent("Collidable")) {
        return;
    }
    boost::optional<EntityAABB> boundingBox = Collision::EntityAbsoluteAABB(entity);
    if (!boundingBox) {
        return;
    }

    ComponentWrapper& cTransform = entity["Transform"];
    EntityAABB& boxA = *boundingBox;
    bool everHitTheGround = false;

    if (entity == LocalPlayer) {
        auto prevPosIt = m_PrevPositions.find(entity);
        if (prevPosIt != m_PrevPositions.end()) {
            glm::vec3 size = boxA.Size();
            float diameter = std::min(size.x, size.z);
            glm::vec3 prevOrigin = prevPosIt->second;
            glm::vec3 toCurrentPos = boxA.Origin() - prevOrigin;
            float rayLength = glm::length(toCurrentPos) + 0.5f*diameter;
            //If the entity has moved farther than the size of its box, we need to handle it specially.
            if (rayLength > diameter) {
                Ray ray(prevOrigin, toCurrentPos);
                m_OctreeResult.clear();
                m_Octree->ObjectsPossiblyHitByRay(ray, m_OctreeResult);
                for (auto& boxB : m_OctreeResult) {
                    if (boxA.Entity == boxB.Entity) {
                        continue;
                    }
                    bool hit;
                    float dist;
                    if (boxB.Entity.HasComponent("Model")) {
                        if (!((bool)boxB.Entity["Model"]["Visible"])) {
                            // Don't collide against invisible models.
                            continue;
                        }
                        Model* model;
                        std::string res = (std::string)boxB.Entity["Model"]["Resource"];
                        try {
                            model = ResourceManager::Load<Model, true>(res);
                        } catch (const std::exception&) {
                            continue;
                        }
                        float u, v;
                        hit = Collision::RayVsModel(ray, model->m_Vertices, model->m_Indices, TransformSystem::ModelMatrix(boxB.Entity), dist, u, v);
                    } else {
                        hit = Collision::RayVsAABB(ray, boxB, dist);
                    }
                    if (hit && dist < rayLength) {
                        //Set the entity to where it was colliding, minus the maximum box size. 
                        //TODO: Perhaps this should be done slightly more properly.
                        glm::vec3 newOriginPos = ray.Origin() + (dist - 0.707107f*diameter) * ray.Direction();
                        glm::vec3 resolve = newOriginPos - boxA.Origin();
                        (Field<glm::vec3>)cTransform["Position"] += resolve;
                        boxA = *Collision::EntityAbsoluteAABB(entity);
                        if (resolve.y > 0) {
                            everHitTheGround = true;
                            cPhysics["IsOnGround"] = true;
                            ((Field<glm::vec3>)cPhysics["Velocity"]).y(0.f);
                        }
                        break;
                    }
                }
            }
        }

        // Collide against octree items
        m_OctreeResult.clear();
        m_Octree->ObjectsInSameRegion(*boundingBox, m_OctreeResult);
        for (auto& boxB : m_OctreeResult) {
            glm::vec3 resolutionVector;
            if (boxA.Entity == boxB.Entity) {
                continue;
            }

            if (boxB.Entity.HasComponent("Model") && Collision::AABBVsAABB(boxA, boxB)) {
                // Here we know boxB is a entity with Collideable, AABB, and Model.
                if (!((const bool&)boxB.Entity["Model"]["Visible"])) {
                    // Don't collide against invisible models.
                    continue;
                }
                Model* model;
                try {
                    model = ResourceManager::Load<Model, true>(boxB.Entity["Model"]["Resource"]);
                } catch (const std::exception&) {
                    continue;
                }

                glm::mat4 modelMatrix = TransformSystem::ModelMatrix(boxB.Entity);

                glm::vec3 inOutVelocity = (glm::vec3)cPhysics["Velocity"];
                bool isOnGround = (bool)cPhysics["IsOnGround"];
                float verticalStepHeight = (float)(double)cPhysics["VerticalStepHeight"];
                if (Collision::AABBvsTriangles(boxA, model->m_Vertices, model->m_Indices, modelMatrix, inOutVelocity, verticalStepHeight, isOnGround, resolutionVector)) {
                    //Move the position to previous position if it is not moving in the xz-plane, else resolve with the resolution vector.
                    (Field<glm::vec3>)cTransform["Position"] += resolutionVector;
                    boxA = *Collision::EntityAbsoluteAABB(entity);
                    cPhysics["Velocity"] = inOutVelocity;
                    if (isOnGround) {
                        everHitTheGround = true;
                        (bool)cPhysics["IsOnGround"] = true;
                    }
                }
            } else if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
                //Enter here if boxB has no Model.
                (Field<glm::vec3>)cTransform["Position"] += resolutionVector;
                boxA = *Collision::EntityAbsoluteAABB(entity);
                if (resolutionVector.y > 0) {
                    everHitTheGround = true;
                    (bool)cPhysics["IsOnGround"] = true;
                    ((Field<glm::vec3>)cPhysics["Velocity"]).y(0.f);
                }
            }
        }

        //This should apply air friction and such, iff zero models were hit.
        if (!everHitTheGround) {
            (bool)cPhysics["IsOnGround"] = false;
        }

        m_PrevPositions[entity] = boxA.Origin();
    }
}
