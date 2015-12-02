#ifndef Collision_h__
#define Collision_h__

#include "../GLM.h"
#include <algorithm>

namespace Collision
{
struct Ray
{
public:
    glm::vec3 Origin;
    glm::vec3 Direction;
};

class AABB
{
public:
    AABB() = default;
    AABB(const glm::vec3& minPos, const glm::vec3& maxPos);

    const glm::vec3& MinCorner() const { return m_MinCorner; }
    const glm::vec3& MaxCorner() const { return m_MaxCorner; }
    const glm::vec3& Center() const { return m_Center; }
    const glm::vec3& HalfSize() const { return m_HalfSize; }
private:
    glm::vec3 m_MinCorner;
    glm::vec3 m_MaxCorner;
    glm::vec3 m_Center;
    glm::vec3 m_HalfSize;
};

bool RayAABBIntr(const Ray& ray, const AABB& box);
bool RayVsAABB(const Ray& ray, const AABB& box);
}

#endif