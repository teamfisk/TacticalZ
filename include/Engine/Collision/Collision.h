#ifndef Collision_h__
#define Collision_h__

//NOTE: Collision.h needs to be #included before <GLFW/glfw3.h>, 
//because Collision #includes "RawModel.h", which has "Texture.h", which has "OpenGL.h" which must be #included first
//or you will get "fatal error C1189: #error:  gl.h included before glew.h"

#include <vector>
#include <boost/optional.hpp>

#include "../Core/Ray.h"
#include "../Core/AABB.h"
#include "Rendering/RawModelCustom.h"
//#include "Rendering/RawModelAssimp.h"
#include "../Core/Transform.h"
#include "../Core/Entity.h"
#include "../Core/EntityWrapper.h"
#include "EntityAABB.h"

class World;
struct ComponentWrapper;

template<typename T>
class Octree;

namespace Collision
{
//Return true if the ray hits the box.
bool RayAABBIntr(const Ray& ray, const AABB& box);
bool RayVsAABB(const Ray& ray, const AABB& box);
//Return true if the ray hits the box, also outputs distance from ray origin to intersection point in [outDistance].
bool RayVsAABB(const Ray& ray, const AABB& box, float& outDistance);

//Return true if the ray hits the triangle.
bool RayVsTriangle(const Ray& ray,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    bool trueOnNegativeDistance = false);
//Return true if the ray hits the triangle, and the distance is less than outDistance.
bool RayVsTriangle(const Ray& ray,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    float& outDistance,
    float& outUCoord,
    float& outVCoord,
    bool trueOnNegativeDistance = false);
//Return true if the ray hits any of the triangles in the model. Stops checking when a hit is detected.
bool RayVsModel(const Ray& ray,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix);
//Return true if the ray hits any of the triangles in the model.
//Also returns the position of the intersection point. Will loop through all the whole model indices.
bool RayVsModel(const Ray& ray,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix,
    glm::vec3& outHitPosition);
//Return true if the ray hits any of the triangles in the model.
//Also returns the distance from the ray origin to the closest 
//intersection point, and the barycentric u,v-coordinates. Will loop through all the whole model indices.
bool RayVsModel(const Ray& ray,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix,
    float& outDistance,
    float& outUCoord,
    float& outVCoord);

bool AABBvsTriangles(const AABB& box,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix,
    glm::vec3& boxVelocity,
    float verticalStepHeight,
    bool& isOnGround,
    glm::vec3& outResolutionVector);

//Return true if the boxes are intersecting.
bool AABBVsAABB(const AABB& a, const AABB& b);
//Return true if the boxes are intersecting.
//Also outputs the minimum translation that box [a] would need in order to resolve collision.
bool AABBVsAABB(const AABB& a, const AABB& b, glm::vec3& minimumTranslation);

// Calculates an absolute AABB from an entity AABB component
boost::optional<EntityAABB> EntityAbsoluteAABB(EntityWrapper& entity, bool takeModelBox = false);
boost::optional<EntityAABB> AbsoluteAABBExplosionEffect(EntityWrapper& entity);
//Returns the first entity hit by the input ray. entitiesPotentiallyHitSorted needs to be sorted 
//by their distance to the ray, e.g. result from Octree<EntityAABB>::ObjectsPossiblyHitByRay.
//Returns boost::none if none was hit. outDistance will be the distance to the intersection point if the ray intersects.
boost::optional<EntityAABB> EntityFirstHitByRay(const Ray& ray, std::vector<EntityAABB> entitiesPotentiallyHitSorted, float& outDistance, glm::vec3& outIntersectPos);
//Returns the first entity hit by the input ray that exists in the octree.
//outDistance will be the distance to the intersection point if the ray intersects. 
boost::optional<EntityAABB> EntityFirstHitByRay(const Ray& ray, Octree<EntityAABB>* octree, float& outDistance, glm::vec3& outIntersectPos);

}

#endif