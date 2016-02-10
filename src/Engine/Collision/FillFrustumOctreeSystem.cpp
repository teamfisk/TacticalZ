#include "Collision/FillFrustumOctreeSystem.h"

void FillFrustumOctreeSystem::Update(double dt)
{
    m_Octree->ClearDynamicObjects();
}

void FillFrustumOctreeSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt)
{
    boost::optional<EntityAABB> absoluteAABB;
    if (entity.HasComponent("ExplosionEffect")) {
        absoluteAABB = Collision::AbsoluteAABBExplosionEffect(entity);
    } else {
        absoluteAABB = Collision::EntityAbsoluteAABB(entity);
    }
    if (absoluteAABB) {
        m_Octree->AddDynamicObject(*absoluteAABB);
    }
}
