#include <algorithm>

#include "Collision/Collision.h"
#include "Engine/GLM.h"
#include "Core/World.h"
#include "Rendering/Model.h"

namespace Collision
{

//note: this one hasnt been delta adjusted like RayVsAABB has
bool RayAABBIntr(const Ray& ray, const AABB& box)
{
    glm::vec3 w = 75.0f * ray.Direction();
    glm::vec3 v = glm::abs(w);
    glm::vec3 c = ray.Origin() - box.Origin() + w;
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
    glm::vec3 invdir = 1.0f / ray.Direction();
    glm::vec3 origin = ray.Origin();

    float t1 = (box.MinCorner().x - origin.x)*invdir.x;
    float t2 = (box.MaxCorner().x - origin.x)*invdir.x;
    float t3 = (box.MinCorner().y - origin.y)*invdir.y;
    float t4 = (box.MaxCorner().y - origin.y)*invdir.y;
    float t5 = (box.MinCorner().z - origin.z)*invdir.z;
    float t6 = (box.MaxCorner().z - origin.z)*invdir.z;

    float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
    float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

    //if (tmax < 0 || tmin > tmax)
    //if tmin,tmax are almost the same (i.e. hitting exactly in the corner) then tmin might be slightly 
    //greater than tmax becuase of floating-precision problems. fixed by adding a small delta to tmax
    if (tmax < 0 || tmin>(tmax + 0.0001f))
        return false;

    outDistance = (tmin > 0) ? tmin : tmax;
    return true;
}

bool AABBVsAABB(const AABB& a, const AABB& b)
{
    const glm::vec3& aCenter = a.Origin();
    const glm::vec3& bCenter = b.Origin();
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

bool AABBVsAABB(const AABB& a, const AABB& b, glm::vec3& minimumTranslation)
{
    minimumTranslation = glm::vec3(0, 0, 0);
    const glm::vec3& aMax = a.MaxCorner();
    const glm::vec3& bMax = b.MaxCorner();
    const glm::vec3& aMin = a.MinCorner();
    const glm::vec3& bMin = b.MinCorner();
    const glm::vec3& bSize = b.Size();
    const glm::vec3& aSize = a.Size();
    float minOffset = INFINITY;
    float off;
    auto axisesIntersecting = glm::tvec3<bool, glm::highp>(false, false, false);
    for (int i = 0; i < 3; ++i) {
        off = bMax[i] - aMin[i];
        if (off > 0 && off < bSize[i] + aSize[i]) {
            if (off < minOffset) {
                minimumTranslation = glm::vec3();
                minimumTranslation[i] = minOffset = off;
            }
            axisesIntersecting[i] = true;
        }
        off = aMax[i] - bMin[i];
        if (off > 0 && off < bSize[i] + aSize[i]) {
            if (off < minOffset) {
                minOffset = off;
                minimumTranslation = glm::vec3();
                minimumTranslation[i] = -off;
            }
            axisesIntersecting[i] = true;
        }
    }
    return glm::all(axisesIntersecting);
}

bool RayVsModel(const Ray& ray,
    const std::vector<RawModel::Vertex>& modelVertices,
    const std::vector<unsigned int>& modelIndices)
{
    for (int i = 0; i < modelIndices.size(); ++i) {
        glm::vec3 v0 = modelVertices[modelIndices[i]].Position;
        glm::vec3 e1 = modelVertices[modelIndices[++i]].Position - v0;		//v1 - v0
        glm::vec3 e2 = modelVertices[modelIndices[++i]].Position - v0;		//v2 - v0
        glm::vec3 m = ray.Origin() - v0;
        glm::vec3 MxE1 = glm::cross(m, e1);
        glm::vec3 DxE2 = glm::cross(ray.Direction(), e2);
        float DetInv = glm::dot(e1, DxE2);
        if (std::abs(DetInv) < FLT_EPSILON) {
            continue;
        }
        DetInv = 1.0f / DetInv;
        float u = glm::dot(m, DxE2) * DetInv;
        float v = glm::dot(ray.Direction(), MxE1) * DetInv;
        //u,v can be very close to 0 but still negative sometimes. added a deltafactor to compensate for that problem
        if ((u + 0.001f) < 0 || (v + 0.001f) < 0 || 1 < u + v) {
            continue;
        }
        //Here, u and v are positive, u+v <= 1, and if distance is positive - triangle is hit.
        if (0 <= glm::dot(e2, MxE1) * DetInv) {
            return true;
        }
    }
    return false;
}

bool RayVsTriangle(const Ray& ray,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    float& outDistance,
    float& outUCoord,
    float& outVCoord,
    bool trueOnNegativeDistance)
{
    glm::vec3 e1 = v1 - v0;		//v1 - v0
    glm::vec3 e2 = v2 - v0;		//v2 - v0
    glm::vec3 m = ray.Origin() - v0;
    glm::vec3 MxE1 = glm::cross(m, e1);
    glm::vec3 DxE2 = glm::cross(ray.Direction(), e2);//pVec
    float DetInv = glm::dot(e1, DxE2);
    if (std::abs(DetInv) < FLT_EPSILON) {
        return false;
    }
    DetInv = 1.0f / DetInv;
    float dist = glm::dot(e2, MxE1) * DetInv;
    if (dist >= outDistance) {
        return false;
    }
    outDistance = dist;
    outUCoord = glm::dot(m, DxE2) * DetInv;
    outVCoord  = glm::dot(ray.Direction(), MxE1) * DetInv;

    //u,v can be very close to 0 but still negative sometimes. added a deltafactor to compensate for that problem
    //If u and v are positive, u+v <= 1, dist is positive, and less than closest.
    return (0 <= (outUCoord + 0.001f) && 0 <= (outVCoord + 0.001f) && outUCoord + outVCoord <= 1 && (trueOnNegativeDistance || 0 <= dist));
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
        glm::vec3 v1 = modelVertices[modelIndices[++i]].Position;
        glm::vec3 v2 = modelVertices[modelIndices[++i]].Position;
        float dist;
        float u;
        float v;
        if (RayVsTriangle(ray, v0, v1, v2, dist, u, v)) {
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
    outHitPosition = ray.Origin() + dist * ray.Direction();
    return hit;
}

constexpr inline int signNonZero(float x)
{
    return x < 0 ? -1 : 1;
}

inline glm::vec3 signNonZero(const glm::vec3& x)
{
    glm::vec3 r;
    for (int i = 0; i < 3; ++i) {
        r[i] = signNonZero(x[i]);
    }
    return r;
}


bool vectorHasLength(const glm::vec3& vec)
{
    return glm::any(glm::greaterThan(glm::abs(vec), glm::vec3(0.0001f, 0.0001f, 0.0001f)));
}

bool AABBvsTriangle(const AABB& box, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, glm::vec3& outResolutionVector)
{
    //Check so we don't have a zero area triangle when calculating the normal.
    glm::vec3 triNormal = glm::cross(v1 - v0, v2 - v0);
    if (!vectorHasLength(triNormal)) {
        return false;
    }

    const glm::vec3& origin = box.Origin();
    const glm::vec3& half = box.HalfSize();
    const glm::vec3& min = box.MinCorner();
    const glm::vec3& max = box.MaxCorner();

    const glm::vec3 triPos[] = {
        v0, v1, v2
    };

    auto insideAllPlanes = glm::tvec3<bool>(false);
    //All triangle vertex points.
    //Check if the triangle is completely outside or inside the box.
    for (int ax = 0; ax < 3; ++ax) {
        auto outsideMinPlane = glm::tvec3<bool>(false);
        auto outsideMaxPlane = glm::tvec3<bool>(false);
        auto insidePlanes = glm::tvec3<bool>(false);
        for (int pos = 0; pos < 3; ++pos) {
            outsideMinPlane[pos] = (min[ax] > triPos[pos][ax]);
            outsideMaxPlane[pos] = (triPos[pos][ax] > max[ax]);
            insidePlanes[pos] = !(outsideMinPlane[pos] || outsideMaxPlane[pos]);
        }
        if (glm::all(outsideMinPlane) || glm::all(outsideMaxPlane)) {
            return false;
        }
        insideAllPlanes[ax] = glm::all(insidePlanes);
    }
    if (glm::all(insideAllPlanes)) {
        outResolutionVector = glm::vec3(0.f);
        LOG_DEBUG("Triangle collision inside");
        return true;    //TODO: Resolve.
    }

    Ray ray;
    //All triangle lines.
    //Check if each line on the triangle intersects any plane on the box.
    for (int l = 0; l < 3; ++l) {
        glm::vec3 edge = triPos[(l + 1) % 3] - triPos[l];
        ray.SetOrigin(triPos[l]);
        ray.SetDirection(edge);
        float dist;
        if (RayVsAABB(ray, box, dist) && dist <= glm::length(edge)) {
            outResolutionVector = glm::vec3(0.f);
            LOG_DEBUG("Triangle collision lines");
            return true;    //TODO: Resolve.
        }
    }

    //None of the polygon's edges intersects the cube, finally, check if any of the four 
    //cube diagonals intersect the interior of the polygon.
    //If the polygon does intersect any of the cube diagonals, it will 
    //intersect the cube diagonal that comes
    //closest to being perpendicular to the plane of the polygon.

    triNormal = glm::normalize(triNormal);
    glm::vec3 diagonal = signNonZero(triNormal) * half;
#define EARLY_OUT_OR_MAYBE_JUST_EXTRA_WORK
#ifdef EARLY_OUT_OR_MAYBE_JUST_EXTRA_WORK
    //The triangle plane contains all points P in dot(triNormal, P) == dot(triNormal, v0)
    //The diagonal line contains all points P in P = origin + diagonal * t.
    float t = glm::dot(triNormal, v0 - origin) / glm::dot(triNormal, diagonal);

    //If intersection point between plane and diagonal is not within the box.
    if (glm::abs(t) > 1) {
        return false;
    }
    //glm::vec3 intersection = origin + t * diagonal;
#endif

    //Check if intersection point is on the triangle.
    ray.SetOrigin(origin);
    ray.SetDirection(diagonal);
    float dist = INFINITY, u, v;
    if (RayVsTriangle(ray, v0, v1, v2, dist, u, v, true)) {
#ifndef EARLY_OUT_OR_MAYBE_JUST_EXTRA_WORK
        if (glm::abs(dist) > glm::length(diagonal)) {
            return false;
        }
#endif
        float dist = glm::dot(triNormal, origin + diagonal - v0);
        outResolutionVector = dist * triNormal;
        LOG_DEBUG("Triangle collision corner");
        return true;
    }
    return false;
}

bool AABBvsTriangles(const AABB& box, const std::vector<RawModel::Vertex>& modelVertices, const std::vector<unsigned int>& modelIndices, const glm::mat4& modelMatrix, glm::vec3& outResolutionVector)
{
    bool hit = false;
    outResolutionVector = glm::vec3(INFINITY);
    for (int i = 0; i < modelIndices.size(); i += 3) {
        glm::vec3 resVec;
        if (AABBvsTriangle(
            box, 
            Transform::TransformPoint(modelVertices[modelIndices[i]].Position, modelMatrix), 
            Transform::TransformPoint(modelVertices[modelIndices[i + 1]].Position, modelMatrix),
            Transform::TransformPoint(modelVertices[modelIndices[i + 2]].Position, modelMatrix),
            resVec
            ))
        {
            hit = true;
            //If resolution distance is smaller than previous, and is non-zero.
            if (glm::length(resVec) < glm::length(outResolutionVector) && vectorHasLength(resVec)) {
                outResolutionVector = resVec;
            }
        }
    }
    //TODO: Unnecessary later, remove it.
    if (glm::any(glm::isinf(outResolutionVector))) {
        outResolutionVector = glm::vec3(0, 0, 0);
    }
    return hit;
}

bool IsSameBoxProbably(const AABB& first, const AABB& second, const float epsilon)
{
    const glm::vec3& ma1 = first.MaxCorner();
    const glm::vec3& ma2 = second.MaxCorner();
    const glm::vec3& mi1 = first.MinCorner();
    const glm::vec3& mi2 = second.MinCorner();
    return (std::abs(ma1.x - ma2.x) < epsilon) &&
        (std::abs(mi1.x - mi2.x) < epsilon) &&
        (std::abs(ma1.z - ma2.z) < epsilon) &&
        (std::abs(mi1.z - mi2.z) < epsilon) &&
        (std::abs(ma1.y - ma2.y) < epsilon) &&
        (std::abs(mi1.y - mi2.y) < epsilon);
}

bool attachAABBComponentFromModel(World* world, EntityID id)
{
    if (!world->HasComponent(id, "Model")) {
        return false;
    }
    ComponentWrapper model = world->GetComponent(id, "Model");
    ComponentWrapper collision = world->AttachComponent(id, "AABB");
    Model* modelRes = ResourceManager::Load<Model>(model["Resource"]);
    if (modelRes == nullptr) {
        return false;
    }

    glm::mat4 modelMatrix = modelRes->Matrix();

    glm::vec3 mini = glm::vec3(INFINITY, INFINITY, INFINITY);
    glm::vec3 maxi = glm::vec3(-INFINITY, -INFINITY, -INFINITY);
    for (const auto& v : modelRes->Vertices()) {
        const auto& wPos = modelMatrix * glm::vec4(v.Position.x, v.Position.y, v.Position.z, 1);
        maxi.x = std::max(wPos.x, maxi.x);
        maxi.y = std::max(wPos.y, maxi.y);
        maxi.z = std::max(wPos.z, maxi.z);
        mini.x = std::min(wPos.x, mini.x);
        mini.y = std::min(wPos.y, mini.y);
        mini.z = std::min(wPos.z, mini.z);
    }
    collision["Origin"] = 0.5f * (maxi + mini);
    collision["Size"] = maxi - mini;
    return true;
}

boost::optional<AABB> EntityAbsoluteAABB(EntityWrapper& entity)
{
    if (!entity.HasComponent("AABB")) {
        return boost::none;
    }

    ComponentWrapper& cAABB = entity["AABB"];
    glm::vec3 absPosition = Transform::AbsolutePosition(entity.World, entity.ID);
    glm::vec3 absScale = Transform::AbsoluteScale(entity.World, entity.ID);
    glm::vec3 origin = absPosition + (glm::vec3)cAABB["Origin"];
    glm::vec3 size = (glm::vec3)cAABB["Size"] * absScale;
    return AABB::FromOriginSize(origin, size);
}

}
