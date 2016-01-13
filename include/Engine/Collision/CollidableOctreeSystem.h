#include "../Core/System.h"
#include "../Core/Octree.h"
#include "Collision.h"

class CollidableOctreeSystem : public ImpureSystem, public PureSystem
{
public:
    CollidableOctreeSystem(EventBroker* eventBroker, Octree* octree)
        : System(eventBroker)
        , PureSystem("Collidable")
        , m_Octree(octree)
    { }

    virtual void Update(World* world, double dt) override;
    virtual void UpdateComponent(World* world, ComponentWrapper& component, double dt) override;

private:
    Octree* m_Octree;
};