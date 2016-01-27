#include "Collision/CollidableOctreeSystem.h"

void CollidableOctreeSystem::Update(double dt)
{
    m_Octree->ClearDynamicObjects();
}

void CollidableOctreeSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    if (entity.HasComponent("AABB")) {
        boost::optional<EntityAABB> absoluteAABB = Collision::EntityAbsoluteAABB(entity);
        if (absoluteAABB) {
            m_Octree->AddDynamicObject(*absoluteAABB);
        }
    } else if (entity.HasComponent("Model")) {
        // TODO: Derive AABB from model
    }
}