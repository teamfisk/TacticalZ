#include "Core/Collision.h"
#include "Engine/GLM.h"

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

}