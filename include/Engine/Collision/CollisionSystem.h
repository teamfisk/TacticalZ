#ifndef CollisionSystem_h__
#define CollisionSystem_h__

#include <GLFW/glfw3.h>
#include <glm/common.hpp>

#include "../Common.h"
#include "../Core/System.h"
#include "../Core/EventBroker.h"
#include "../Core/Octree.h"
#include "EntityAABB.h"

class CollisionSystem : public PureSystem
{
public:
    CollisionSystem(SystemParams params, Octree<EntityAABB>* octree)
        : System(params)
        , PureSystem("Physics")
        , m_Octree(octree)
    { }

    virtual void UpdateComponent(EntityWrapper& entity, ComponentWrapper& cPhysics, double dt) override;

private:
    Octree<EntityAABB>* m_Octree;
    std::vector<EntityAABB> m_OctreeResult;
    std::unordered_map<EntityWrapper, glm::vec3> m_PrevPositions;
};

#endif