#ifndef EntityAABB_h__
#define EntityAABB_h__

#include "../Core/AABB.h"
#include "../Core/EntityWrapper.h"

struct EntityAABB : AABB
{
    EntityAABB() = default;

    EntityAABB(const glm::vec3& minPos, const glm::vec3& maxPos)
        : AABB(minPos, maxPos)
    { }

    EntityAABB(const AABB& aabb)
        : AABB(aabb)
    { }

    EntityWrapper Entity;
};

#endif