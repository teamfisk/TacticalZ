#include "Collision/FillOctreeSystem.h"

void FillOctreeSystem::Update(double dt)
{
    m_Octree->ClearDynamicObjects();
}

void FillOctreeSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
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