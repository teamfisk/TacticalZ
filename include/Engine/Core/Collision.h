#ifndef Collision_h__
#define Collision_h__

#include <vector>

#include "Core/Ray.h"
#include "Core/AABB.h"
#include "Engine/Rendering/RawModel.h"

namespace Collision
{
//Return true if the ray hits the box.
bool RayAABBIntr(const Ray& ray, const AABB& box);
bool RayVsAABB(const Ray& ray, const AABB& box);
//Return true if the ray hits the box, also outputs distance from ray origin to intersection point in [outDistance].
bool RayVsAABB(const Ray& ray, const AABB& box, float& outDistance);

//Return true if the ray hits any of the triangles in the model. Stops checking when a hit is detected.
bool RayVsModel(const Ray& ray, 
    const std::vector<RawModel::Vertex>& modelVertices,
    const std::vector<unsigned int>& modelIndices);
//Return true if the ray hits any of the triangles in the model.
//Also returns the position of the intersection point. Will loop through all the whole model indices.
bool RayVsModel(const Ray& ray,
    const std::vector<RawModel::Vertex>& modelVertices,
    const std::vector<unsigned int>& modelIndices,
    glm::vec3& outHitPosition);
//Return true if the ray hits any of the triangles in the model.
//Also returns the distance from the ray origin to the closest 
//intersection point, and the barycentric u,v-coordinates. Will loop through all the whole model indices.
bool RayVsModel(const Ray& ray,
    const std::vector<RawModel::Vertex>& modelVertices,
    const std::vector<unsigned int>& modelIndices,
    float& outDistance,
    float& outUCoord,
    float& outVCoord);

//Return true if the boxes are intersecting.
bool AABBVsAABB(const AABB& a, const AABB& b);

}

#endif