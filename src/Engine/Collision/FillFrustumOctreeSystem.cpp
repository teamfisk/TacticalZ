#include "Collision/FillFrustumOctreeSystem.h"

void FillFrustumOctreeSystem::Update(double dt)
{
    m_Octree->ClearDynamicObjects();
}

void FillFrustumOctreeSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    if (entity.HasComponent("ExplosionEffect")) {
        //TODO: Fix hack, get real box by using shader equation.
        EntityAABB aabb = AABB(glm::vec3(-300), glm::vec3(300));
        aabb.Entity = entity;
        m_Octree->AddDynamicObject(aabb);
    } else {
        boost::optional<EntityAABB> absoluteAABB = Collision::EntityAbsoluteAABB(entity);
        if (absoluteAABB) {
            m_Octree->AddDynamicObject(*absoluteAABB);
        }
    }
}