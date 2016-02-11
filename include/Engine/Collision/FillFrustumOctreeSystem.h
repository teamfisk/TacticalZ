#ifndef FillFrustumOctreeSystem_h__
#define FillFrustumOctreeSystem_h__

#include "../Core/System.h"
#include "../Core/Octree.h"
#include "Collision.h"
#include "EntityAABB.h"

class FillFrustumOctreeSystem : public ImpureSystem, public PureSystem
{
public:
    FillFrustumOctreeSystem(SystemParams params, Octree<EntityAABB>* octree)
        : System(params)
        , PureSystem("Model")
        , m_Octree(octree)
    { }

    virtual void Update(double dt) override;
    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    Octree<EntityAABB>* m_Octree;
};

#endif