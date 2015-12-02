#include "Core\Collision.h"
#include <algorithm>

namespace Collision
{

AABB::AABB(const glm::vec3& minPos, const glm::vec3& maxPos)
    : m_MinCorner(minPos)
    , m_MaxCorner(maxPos)
    , m_Center(0.5f * (maxPos + minPos))
    , m_HalfSize(0.5f * (maxPos - minPos))
{ }

bool RayAABBIntr(const Ray& ray, const AABB& box)
{
    glm::vec3 w = 75.0f * ray.Direction;
    glm::vec3 v = glm::abs(w);
    glm::vec3 c = ray.Origin - box.Center() + w;
    glm::vec3 half = box.HalfSize();
    
    if (abs(c.x) > v.x + half.x) {
        return false;
    }
    if (abs(c.y) > v.y + half.y) {
        return false;
    }
    if (abs(c.z) > v.z + half.z) {
        return false;
    }

    if (abs(c.y*w.z - c.z*w.y) > half.y*v.z + half.z*v.y) {
        return false;
    }
    if (abs(c.x*w.z - c.z*w.x) > half.x*v.z + half.z*v.x) {
        return false;
    }
    return !(abs(c.x*w.y - c.y*w.x) > half.x*v.y + half.y*v.x);
}

bool RayVsAABB(const Ray& ray, const AABB& box)
{
    glm::vec3 invdir = 1.0f / ray.Direction;
    glm::vec3 bMin = box.MinCorner();
    glm::vec3 bMax = box.MaxCorner();
    
    float t1 = (bMin.x - ray.Origin.x)*invdir.x;
    float t2 = (bMax.x - ray.Origin.x)*invdir.x;
    float t3 = (bMin.y - ray.Origin.y)*invdir.y;
    float t4 = (bMax.y - ray.Origin.y)*invdir.y;
    float t5 = (bMin.z - ray.Origin.z)*invdir.z;
    float t6 = (bMax.z - ray.Origin.z)*invdir.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    if (tmax < 0 || tmin > tmax)
        return false;

    return true;
}

}