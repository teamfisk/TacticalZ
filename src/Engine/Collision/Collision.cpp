#include <algorithm>

#include "Collision/Collision.h"
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

bool RayVsAABB(const Ray& ray, const AABB& box)
{
    float dummy;
    return RayVsAABB(ray, box, dummy);
}

bool RayVsAABB(const Ray& ray, const AABB& box, float& outDistance)
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

    outDistance = (tmin > 0) ? tmin : tmax;
    return true;
}

bool AABBVsAABB(const AABB& a, const AABB& b)
{
    const glm::vec3& aCenter = a.Center();
    const glm::vec3& bCenter = b.Center();
    const glm::vec3& aHSize = a.HalfSize();
    const glm::vec3& bHSize = b.HalfSize();
    //Test will probably exit because of the X and Z axes more often, so test them first.
    if (abs(aCenter[0] - bCenter[0]) > (aHSize[0] + bHSize[0])) {
        return false;
    }
    if (abs(aCenter[2] - bCenter[2]) > (aHSize[2] + bHSize[2])) {
        return false;
    }
    return (abs(aCenter[1] - bCenter[1]) <= (aHSize[1] + bHSize[1]));
}

bool RayVsModel(const Ray& ray, 
    const std::vector<RawModel::Vertex>& modelVertices, 
    const std::vector<unsigned int>& modelIndices)
{
    for (int i = 0; i < modelIndices.size(); ++i) {
        glm::vec3 v0 = modelVertices[modelIndices[i]].Position;
        glm::vec3 e1 = modelVertices[modelIndices[++i]].Position - v0;		//v1 - v0
        glm::vec3 e2 = modelVertices[modelIndices[++i]].Position - v0;		//v2 - v0
        glm::vec3 m = ray.Origin - v0;
        glm::vec3 MxE1 = glm::cross(m, e1);
        glm::vec3 DxE2 = glm::cross(ray.Direction, e2);
        float DetInv = glm::dot(e1, DxE2);
        if (std::abs(DetInv) < FLT_EPSILON) {
            continue;
        }
        DetInv = 1.0f / DetInv;
        float u = glm::dot(m, DxE2) * DetInv;
        float v = glm::dot(ray.Direction, MxE1) * DetInv;
        if (u < 0 || v < 0 || 1 < u + v) {
            continue;
        }
        //Here, u and v are positive, u+v <= 1, and if distance is positive - triangle is hit.
        if (0 <= glm::dot(e2, MxE1) * DetInv) {
            return true;
        }
    }
    return false;
}

bool RayVsModel(const Ray& ray,
    const std::vector<RawModel::Vertex>& modelVertices,
    const std::vector<unsigned int>& modelIndices,
    float& outDistance,
    float& outUCoord,
    float& outVCoord)
{
    outDistance = INFINITY;
    bool hit = false;
    for (int i = 0; i < modelIndices.size(); ++i) {
        glm::vec3 v0 = modelVertices[modelIndices[i]].Position;
        glm::vec3 e1 = modelVertices[modelIndices[++i]].Position - v0;		//v1 - v0
        glm::vec3 e2 = modelVertices[modelIndices[++i]].Position - v0;		//v2 - v0
        glm::vec3 m = ray.Origin - v0;
        glm::vec3 MxE1 = glm::cross(m, e1);
        glm::vec3 DxE2 = glm::cross(ray.Direction, e2);
        float DetInv = glm::dot(e1, DxE2);
        if (std::abs(DetInv) < FLT_EPSILON) {
            continue;
        }
        float dist = glm::dot(e2, MxE1) * DetInv;
        if (dist >= outDistance) {
            continue;
        }
        float u = glm::dot(m, DxE2) * DetInv;
        float v = glm::dot(ray.Direction, MxE1) * DetInv;
        //If u and v are positive, u+v <= 1, dist is positive, and less than closest.
        if (0 <= u && 0 <= v && u + v <= 1 && 0 <= dist) {
            outDistance = dist;
            outUCoord = u;
            outVCoord = v;
            hit = true;
        }
    }
    return hit;
}

bool RayVsModel(const Ray& ray,
    const std::vector<RawModel::Vertex>& modelVertices,
    const std::vector<unsigned int>& modelIndices,
    glm::vec3& outHitPosition)
{
    float u;
    float v;
    float dist;
    bool hit = RayVsModel(ray, modelVertices, modelIndices, dist, u, v);
    outHitPosition = ray.Origin + dist * ray.Direction;
    return hit;
}

}
