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
    //std::vector<AABB> octreeResult;
    //m_Octree->ObjectsInSameRegion(*boundingBox, octreeResult);
    //for (auto& boxB : octreeResult) {
    //    glm::vec3 resolutionVector;
    //    if (Collision::IsSameBoxProbably(boxA, boxB)) {
    //        continue;
    //    }
    //    if (Collision::AABBVsAABB(boxA, boxB, resolutionVector)) {
    //        (glm::vec3&)cTransform["Position"] += resolutionVector;
    //        cPhysics["Velocity"] = glm::vec3(0, 0, 0);
    //    }
    //}

    // HACK: Temporarily collide against all collidable models since they're not in the octree yet
    auto otherCollidables = m_World->GetComponents("Model");
    for (auto& cModel : *otherCollidables) {
        if (cModel.EntityID == entity.ID) {
            continue;
        }
        if (!m_World->HasComponent(cModel.EntityID, "Collidable")) {
            continue;
        }

        auto absPosition = Transform::AbsolutePosition(m_World, cModel.EntityID);
        auto absOrientation = Transform::AbsoluteOrientation(m_World, cModel.EntityID);
        auto absScale = Transform::AbsoluteScale(m_World, cModel.EntityID);
        glm::mat4 modelMatrix = glm::translate(absPosition) * glm::toMat4(absOrientation) * glm::scale(absScale);

        RawModel* model = ResourceManager::Load<RawModel>(cModel["Resource"]);
        glm::vec3 resolutionVector;
        if (Collision::AABBvsTriangles(boxA, model->m_Vertices, model->m_Indices, modelMatrix, resolutionVector)) {
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
