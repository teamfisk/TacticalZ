#include "Core/Collision.h"
#include "Engine/GLM.h"
#include <algorithm>

namespace Collision
{

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

    float t1 = (box.MinCorner().x - ray.Origin.x)*invdir.x;
    float t2 = (box.MaxCorner().x - ray.Origin.x)*invdir.x;
    float t3 = (box.MinCorner().y - ray.Origin.y)*invdir.y;
    float t4 = (box.MaxCorner().y - ray.Origin.y)*invdir.y;
    float t5 = (box.MinCorner().z - ray.Origin.z)*invdir.z;
    float t6 = (box.MaxCorner().z - ray.Origin.z)*invdir.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    if (tmax < 0 || tmin > tmax)
        return false;

    return true;
}

}