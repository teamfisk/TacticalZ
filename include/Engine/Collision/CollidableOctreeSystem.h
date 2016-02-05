#ifndef CollidableOctreeSystem_h__
#define CollidableOctreeSystem_h__

#include "../Core/System.h"
#include "../Core/Octree.h"
#include "Collision.h"
#include "EntityAABB.h"

class CollidableOctreeSystem : public ImpureSystem, public PureSystem
{
public:
    CollidableOctreeSystem(World* world, EventBroker* eventBroker, Octree<EntityAABB>* octree, const std::string& componentType)
        : System(world, eventBroker)
        , PureSystem(componentType)
        , m_Octree(octree)
    { }

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    Octree<EntityAABB>* m_Octree;
};

#endif