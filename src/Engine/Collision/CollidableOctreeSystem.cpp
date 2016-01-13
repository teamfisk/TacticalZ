#include "Collision/CollidableOctreeSystem.h"

void CollidableOctreeSystem::Update(World* world, double dt)
{
    m_Octree->ClearDynamicObjects();
}

void CollidableOctreeSystem::UpdateComponent(World* world, ComponentWrapper& cCollidable, double dt)
{
    EntityID entity = cCollidable.EntityID;

    if (world->HasComponent(entity, "AABB")) {
        boost::optional<AABB> absoluteAABB = Collision::EntityAbsoluteAABB(world, entity);
        if (absoluteAABB) {
            m_Octree->AddDynamicObject(*absoluteAABB);
        }
    } else if (world->HasComponent(entity, "Model")) {
        // TODO: Derive AABB from model
    }
}