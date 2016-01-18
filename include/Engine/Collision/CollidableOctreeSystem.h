#ifndef CollidableOctreeSystem_h__
#define CollidableOctreeSystem_h__

#include "../Core/System.h"
#include "../Core/Octree.h"
#include "Collision.h"

class CollidableOctreeSystem : public ImpureSystem, public PureSystem
{
public:
    CollidableOctreeSystem(World* world, EventBroker* eventBroker, Octree* octree)
        : System(world, eventBroker)
        , PureSystem("Collidable")
        , m_Octree(octree)
    { }

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    Octree* m_Octree;
};

#endif