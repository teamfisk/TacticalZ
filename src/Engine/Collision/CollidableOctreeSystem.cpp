#include "Collision/CollidableOctreeSystem.h"

void CollidableOctreeSystem::Update(World* world, double dt)
{
    m_Octree->ClearDynamicObjects();
}

void CollidableOctreeSystem::UpdateComponent(World* world, EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    if (entity.HasComponent("AABB")) {
        boost::optional<AABB> absoluteAABB = Collision::EntityAbsoluteAABB(entity);
        if (absoluteAABB) {
            m_Octree->AddDynamicObject(*absoluteAABB);
        }
    } else if (entity.HasComponent("Model")) {
        // TODO: Derive AABB from model
    }
}