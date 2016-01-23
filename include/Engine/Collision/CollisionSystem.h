#ifndef CollisionSystem_h__
#define CollisionSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "../Common.h"
#include "../Core/System.h"
#include "../Core/EventBroker.h"
#include "../Core/EKeyUp.h"
#include "../Core/Octree.h"

class CollisionSystem : public PureSystem
{
public:
    CollisionSystem(World* world, EventBroker* eventBroker, Octree<AABB>* octree)
        : System(world, eventBroker)
        , PureSystem("Collidable")
        , m_Octree(octree)
    {
    }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& component, double dt) override;

private:
    Octree<AABB>* m_Octree;
};

#endif