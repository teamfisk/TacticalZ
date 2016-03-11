#include <algorithm>
#include <bitset>

#include "Collision/Collision.h"
#include "Engine/GLM.h"
#include "Core/World.h"
#include "Rendering/Model.h"
#include "imgui/imgui.h"
#include "Core/Octree.h"

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

bool RayVsTriangle(const Ray& ray,
    const glm::vec3& v0,
    const glm::vec3& v1,
    const glm::vec3& v2,
    bool trueOnNegativeDistance)
{
    glm::vec3 e1 = v1 - v0;		//v1 - v0
    glm::vec3 e2 = v2 - v0;		//v2 - v0
    glm::vec3 m = ray.Origin() - v0;
    glm::vec3 MxE1 = glm::cross(m, e1);
    glm::vec3 DxE2 = glm::cross(ray.Direction(), e2);
    float DetInv = glm::dot(e1, DxE2);
    if (std::abs(DetInv) < FLT_EPSILON) {
        return false;
    }
    DetInv = 1.0f / DetInv;
    float u = glm::dot(m, DxE2) * DetInv;
    float v = glm::dot(ray.Direction(), MxE1) * DetInv;
    //u,v can be very close to 0 but still negative sometimes. added a deltafactor to compensate for that problem
    if ((u + 0.001f) < 0 || (v + 0.001f) < 0 || 1 < u + v) {
        return false;
    }
    //Here, u and v are positive, u+v <= 1, and if distance is positive - triangle is hit.
    return trueOnNegativeDistance || 0 <= glm::dot(e2, MxE1) * DetInv;
}

bool RayVsModel(const Ray& ray,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix)
{
    for (int i = 0; i < modelIndices.size();) {
        glm::vec3 v0 = Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix);
        glm::vec3 v1 = Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix);
        glm::vec3 v2 = Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix);
        if (RayVsTriangle(ray, v0, v1, v2)) {
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
    outVCoord = glm::dot(ray.Direction(), MxE1) * DetInv;

    //u,v can be very close to 0 but still negative sometimes. added a deltafactor to compensate for that problem
    //If u and v are positive, u+v <= 1, dist is positive, and less than closest.
    return (0 <= (outUCoord + 0.001f) && 0 <= (outVCoord + 0.001f) && outUCoord + outVCoord <= 1 && (trueOnNegativeDistance || 0 <= dist));
}

bool RayVsModel(const Ray& ray,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix,
    float& outDistance,
    float& outUCoord,
    float& outVCoord)
{
    outDistance = INFINITY;
    bool hit = false;
    for (int i = 0; i < modelIndices.size();) {
        glm::vec3 v0 = Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix);
        glm::vec3 v1 = Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix);
        glm::vec3 v2 = Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix);
        float dist = outDistance;
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
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix,
    glm::vec3& outHitPosition)
{
    float u;
    float v;
    float dist;
    bool hit = RayVsModel(ray, modelVertices, modelIndices, modelMatrix, dist, u, v);
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
        r[i] = (float)signNonZero(x[i]);
    }
    return r;
}

template<typename T>
bool vectorHasLength(const T& vec)
{
    return glm::any(glm::greaterThan(glm::abs(vec), T(0.0001f)));
}

bool rectangleVsTriangle(const glm::vec2& boxMin,
    const glm::vec2& boxMax,
    const std::array<glm::vec2, 3>& triPos,
    glm::vec2& resolutionDirection,
    float& resolutionDistanceSq,
    bool& pushedFromTriNormal)
{
    pushedFromTriNormal = false;
    resolutionDistanceSq = INFINITY;
    //Project along box normals (coordinate axes, since it's axis-aligned).
    for (int ax = 0; ax < 2; ++ax) {
        float minTri = INFINITY;
        float maxTri = -INFINITY;
        for (const glm::vec2& t : triPos) {
            minTri = std::min(t[ax], minTri);
            maxTri = std::max(t[ax], maxTri);
        }
        if (boxMax[ax] <= minTri || maxTri <= boxMin[ax]) {
            return false;
        }

        //Here: maxBox > minTri && minBox < maxTri
        //Left is negative.
        float leftRes = minTri - boxMax[ax];
        float rightRes = maxTri - boxMin[ax];
        float push = rightRes < -leftRes ? rightRes : leftRes;
        float absPushSq = abs(push);
        absPushSq *= absPushSq;

        if (absPushSq < resolutionDistanceSq) {
            resolutionDistanceSq = absPushSq;
            resolutionDirection[1 - ax] = 0.f;
            resolutionDirection[ax] = push;
        }
    }
    //Project along triangle normals.
    //Put edges into normal vector, make normals in the loop.
    std::array<glm::vec2, 3> triNormals = {
        triPos[1] - triPos[0],
        triPos[2] - triPos[1],
        triPos[0] - triPos[2]
    };
    std::array<glm::vec2, 4> boxPos = {
        boxMax,
        glm::vec2(boxMax.x, boxMin.y),
        glm::vec2(boxMin.x, boxMax.y),
        boxMin
    };
    for (auto& normal : triNormals) {
        if (!vectorHasLength(normal)) {
            continue;
        }
        //Rotate edge to a normal.
        normal = glm::normalize(glm::vec2(-normal.y, normal.x));
        //Project triangle onto the normal.
        float minTri = INFINITY;
        float maxTri = -INFINITY;
        for (const glm::vec2& point : triPos) {
            float dot = glm::dot(normal, point);
            minTri = std::min(dot, minTri);
            maxTri = std::max(dot, maxTri);
        }
        //Project box onto the normal.
        float minBox = INFINITY;
        float maxBox = -INFINITY;
        for (const glm::vec2& point : boxPos) {
            float dot = glm::dot(normal, point);
            minBox = std::min(dot, minBox);
            maxBox = std::max(dot, maxBox);
        }
        if (maxBox <= minTri || maxTri <= minBox) {
            return false;
        }

        //Here: maxBox > minTri && minBox < maxTri
        //Left is negative.
        float leftRes = minTri - maxBox;
        float rightRes = maxTri - minBox;
        float push = rightRes < -leftRes ? rightRes : leftRes;
        float absPushSq = abs(push);
        absPushSq *= absPushSq;

        if (absPushSq < resolutionDistanceSq) {
            resolutionDistanceSq = absPushSq;
            resolutionDirection = push * normal;
            pushedFromTriNormal = true;
        }
    }
    return true;
}

constexpr float SlopeConstant(float degrees)
{
    return (1.0f - degrees / 90.f);
}

//Returns true if the angle between horizon and the collision surface is less than 45 degrees. 
constexpr bool FaceIsGround(float faceNormalY)
{
    //TODO: Perhaps the 45 degrees could be saved in a component or in the config..
    return faceNormalY > SlopeConstant(45.0f);
}

//An array containing 3 int pairs { 0, 2 }, { 0, 1 }, { 1, 2 }
constexpr std::array<std::pair<int, int>, 3> dimensionPairs({ std::pair<int, int>(0, 2), std::pair<int, int>(0, 1), std::pair<int, int>(1, 2) });

enum class BoxTriRes
{
    Front,
    Behind,
    Intersect
};

BoxTriRes AABBvsTriangle(const AABB& box,
    const std::array<glm::vec3, 3>& triPos,
    const glm::vec3& originalBoxVelocity,
    float verticalStepHeight,
    bool& isOnGround,
    glm::vec3& boxVelocity,
    glm::vec3& outResolution,
    bool resolveCollision)
{
    //Check so we don't have a zero area triangle when calculating the normal.
    //Also, don't check a triangle facing away from the player.
    //Less checks, and we should be able to walk out from models if we are trapped inside.
    glm::vec3 triNormal = glm::cross(triPos[1] - triPos[0], triPos[2] - triPos[0]);
    if (!vectorHasLength(triNormal) || (glm::dot(triNormal, originalBoxVelocity) > 0)) {
        return BoxTriRes::Behind;
    }
    triNormal = glm::normalize(triNormal);

    enum BoxTriResolveCase
    {
        ResolveDimX,
        ResolveDimY,
        ResolveDimZ,
        Line,       //Box edge colliding with triangle line.
        Corner      //Box corner colliding with the triangle face.
    };
    struct Resolution
    {
        Resolution()
            : DistanceSq(INFINITY)
            , Vector(0.f)
        { }
        BoxTriResolveCase Case;
        float DistanceSq;
        glm::vec3 Vector;
    };
    //The smallest resolution that solves the collision.
    Resolution resolveShortest;
    //The smallest resolution that solves the collision, that resolves upwards.
    Resolution resolveUpwards;
    //If player stands on the ground and collides with a ground triangle, 
    //we might step up onto it if the step is small enough.
    bool canStairStepUp = isOnGround && FaceIsGround(triNormal.y);

    const glm::vec3& origin = box.Origin();
    const glm::vec3& half = box.HalfSize();
    const glm::vec3& min = box.MinCorner();
    const glm::vec3& max = box.MaxCorner();

    // If there is no intersection, whether the box center is in front of or behind the triangle.
    BoxTriRes noIntersection = glm::dot(triNormal, origin - triPos[0]) > 0 ? BoxTriRes::Front : BoxTriRes::Behind;

    //For each projection in xy-, xz-, and yx-planes.
    for (std::pair<int, int> dim : dimensionPairs) {
        //2D Triangle.
        //Project triangle.
        std::array<glm::vec2, 3> t2D = {
            glm::vec2(triPos[0][dim.first], triPos[0][dim.second]),
            glm::vec2(triPos[1][dim.first], triPos[1][dim.second]),
            glm::vec2(triPos[2][dim.first], triPos[2][dim.second])
        };
        //Project box.
        glm::vec2 boxMin(min[dim.first], min[dim.second]);
        glm::vec2 boxMax(max[dim.first], max[dim.second]);
        glm::vec2 resolutionVector;
        float resolutionDist;
        bool pushedFromTriangleLine;
        //if projections don't overlap, return false.
        if (!rectangleVsTriangle(boxMin, boxMax, t2D, resolutionVector, resolutionDist, pushedFromTriangleLine)) {
            return noIntersection;
        } else if (resolveCollision) {
            //Overwrite the smallest resolution if this is smaller.
            if (resolutionDist < resolveShortest.DistanceSq) {
                resolveShortest.Vector = glm::vec3(0.f);
                resolveShortest.Vector[dim.first] = resolutionVector.x;
                resolveShortest.Vector[dim.second] = resolutionVector.y;
                resolveShortest.DistanceSq = resolutionDist;
                //If we pushed away from triangle line (edge), or if we 
                //move the player along one coordinate axis (pick the dimension that isn't zero).
                resolveShortest.Case = pushedFromTriangleLine ? Line : static_cast<BoxTriResolveCase>((abs(resolveShortest.Vector[dim.first]) < 0.0001f) ? dim.second : dim.first);
            }
            //Overwrite the smallest upward resolution if this is smaller, and resolves upwards.
            constexpr int yAxis = 1;
            bool resIsUpwardsIn3D = dim.first == yAxis && resolutionVector.x > 0 || dim.second == yAxis && resolutionVector.y > 0;
            if (canStairStepUp && resIsUpwardsIn3D && resolutionDist < resolveUpwards.DistanceSq) {
                resolveUpwards.Vector = glm::vec3(0.f);
                resolveUpwards.Vector[dim.first] = resolutionVector.x;
                resolveUpwards.Vector[dim.second] = resolutionVector.y;
                resolveUpwards.DistanceSq = resolutionDist;
                //If we pushed away from triangle line (edge), or if we 
                //move the player along one coordinate axis (pick the dimension that isn't zero).
                resolveUpwards.Case = pushedFromTriangleLine ? Line : static_cast<BoxTriResolveCase>((abs(resolveUpwards.Vector[dim.first]) < 0.0001f) ? dim.second : dim.first);
            }
        }
    }

    //If the triangle does intersect any of the cube diagonals, it will 
    //intersect the cube diagonal that comes
    //closest to being perpendicular to the plane of the triangle.
    glm::vec3 diagonal = signNonZero(triNormal) * half;
    //The triangle plane contains all points P in dot(triNormal, P) == dot(triNormal, v0)
    //The diagonal line contains all points P in P = origin + diagonal * t.
    float t = glm::dot(triNormal, triPos[0] - origin) / glm::dot(triNormal, diagonal);
    //If intersection point between plane and diagonal is within the box.
    if (glm::abs(t) > 1) {
        return noIntersection;
    }

    if (!resolveCollision) {
        return BoxTriRes::Intersect;
    }

    glm::vec3 cornerResolution = (1+t) * diagonal;
    cornerResolution = glm::dot(cornerResolution, triNormal) * triNormal;
    //Overwrite the smallest resolution if cornerResolution is smaller.
    float lenSq = glm::length2(cornerResolution);
    if (lenSq < resolveShortest.DistanceSq) {
        resolveShortest.Vector = cornerResolution;
        resolveShortest.Case = Corner;
    }
    if (canStairStepUp && cornerResolution.y > 0 && lenSq < resolveUpwards.DistanceSq) {
        resolveUpwards.Vector = cornerResolution;
        resolveUpwards.Case = Corner;
        resolveUpwards.DistanceSq = lenSq;
    }

    //Force the resolution upwards if it is smaller than the threshold verticalStepHeight.
    //Else take the shortest resolution.
    bool takeUp = resolveUpwards.Vector.y > 0 && resolveUpwards.Vector.y < verticalStepHeight;
    Resolution& bestResolve = takeUp ? resolveUpwards : resolveShortest;
    outResolution = bestResolve.Vector;

    glm::vec3 projNorm;
    switch (bestResolve.Case) {
    case ResolveDimY:
        boxVelocity.y = 0.f;
        if (outResolution.y > 0)
            isOnGround = true;
    case ResolveDimX:
    case ResolveDimZ:
        //If we get here, the resolution is along one coordinate axis.
        //set velocity to 0 in y if it is along y-axis.
        return BoxTriRes::Intersect;
    case Line:
        projNorm = glm::normalize(outResolution);
        break;
    case Corner:
        projNorm = triNormal;
        break;
    default:
        break;
    }

    //If the collision was not on steep wall or similarly (e.g. walking on the ground), force resolution in y only.
    if (FaceIsGround(projNorm.y)) {
        //Ensure that the player always is moved upwards, instead of sliding down.
        float len = glm::length(outResolution);
        float ang = glm::half_pi<float>() - glm::acos(outResolution.y / len);
        if (len > 0.0000001f && ang > 0.0000001f) {
            outResolution.x = 0;
            outResolution.y = len / glm::sin(ang);
            outResolution.z = 0;
        }
        //Also zero the vertical velocity, if it is positive, else project it onto the normal.
        //Project the velocity onto the normal of the hit line/face.
        //w = v - <v,n>*n, |n|==1.
        boxVelocity.y = std::min(boxVelocity.y - glm::dot(boxVelocity, projNorm) * projNorm.y, 0.f);
        isOnGround = true;
    } else {
        //Enter here if the triangle is a steep slope, and it is not facing downwards.
        //Project the velocity onto the normal of the hit line/face.
        //w = v - <v,n>*n, |n|==1.
        //"ice cream"-effect, air resistance + projected velocity.
        if (!isOnGround) {
            boxVelocity = boxVelocity - glm::dot(boxVelocity, projNorm) * projNorm;
        }
    }
    return BoxTriRes::Intersect;
}

Output AABBvsTriangles(const AABB& box,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix,
    glm::vec3& boxVelocity,
    float verticalStepHeight,
    bool& isOnGround,
    glm::vec3& outResolutionVector,
    bool resolveCollision)
{
    bool intersect = false;
    Output out = Output::OutContained;
    bool everHitTheGround = false;
    AABB newBox = box;
    outResolutionVector = glm::vec3(0.f);
    glm::vec3 originalBoxVelocity(boxVelocity);
    for (int i = 0; i < modelIndices.size(); ) {
        std::array<glm::vec3, 3> triVertices = {
            Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix),
            Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix),
            Transform::TransformPoint(modelVertices[modelIndices[i++]].Position, modelMatrix)
        };
        glm::vec3 outVec;
        bool collideWithGround = isOnGround;
        switch (AABBvsTriangle(newBox, triVertices, originalBoxVelocity, verticalStepHeight, collideWithGround, boxVelocity, outVec, resolveCollision)) {
        case Collision::BoxTriRes::Front:
            out = Output::OutSeparated;
            break;
        case Collision::BoxTriRes::Intersect:
            intersect = true;
            outResolutionVector += outVec;
            newBox = AABB::FromOriginSize(newBox.Origin() + outVec, newBox.Size());
            if (collideWithGround) {
                everHitTheGround = isOnGround = true;
            }
            break;
        default:
            break;
        }
    }

    if (!everHitTheGround) {
        isOnGround = false;
    }
    return intersect ? Output::OutIntersecting : out;
}

bool AABBvsTriangles(const AABB& box,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix,
    glm::vec3& boxVelocity,
    float verticalStepHeight,
    bool& isOnGround,
    glm::vec3& outResolutionVector)
{
    return AABBvsTriangles(box,
        modelVertices,
        modelIndices,
        modelMatrix,
        boxVelocity,
        verticalStepHeight,
        isOnGround,
        outResolutionVector,
        true) == Output::OutIntersecting;
}

bool AABBvsTriangles(const AABB& box,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix)
{
    glm::vec3 vel, outres;
    bool g;
    return AABBvsTriangles(box,
        modelVertices,
        modelIndices,
        modelMatrix,
        vel,
        0.f,
        g,
        outres,
        false) == Output::OutIntersecting;
}

Output AABBvsTrianglesWContainment(const AABB& box,
    const RawModel::Vertex* modelVertices,
    const std::vector<unsigned int>& modelIndices,
    const glm::mat4& modelMatrix)
{
    glm::vec3 vel, outres;
    bool g;
    return AABBvsTriangles(box,
        modelVertices,
        modelIndices,
        modelMatrix,
        vel,
        0.f,
        g,
        outres,
        false);
}

boost::optional<EntityAABB> EntityAbsoluteAABB(EntityWrapper& entity, bool takeModelBox)
{
    AABB modelSpaceBox;
    if (entity.HasComponent("AABB") && !takeModelBox) {
        ComponentWrapper& cAABB = entity["AABB"];
        modelSpaceBox = EntityAABB::FromOriginSize((glm::vec3)cAABB["Origin"], (glm::vec3)cAABB["Size"]);
    } else if (entity.HasComponent("Model")) {
        std::string res = entity["Model"]["Resource"];
        if (res.empty()) {
            return boost::none;
        }
        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(res);
        } catch (const Resource::StillLoadingException&) {
            return boost::none;
        } catch (const std::exception&) {
            return boost::none;
        }
        modelSpaceBox = model->Box();
    } else {
        return boost::none;
    }

    glm::mat4 modelMat = Transform::AbsoluteTransformation(entity);
    glm::vec3 mini(INFINITY);
    glm::vec3 maxi(-INFINITY);
    glm::vec3 maxCorner = modelSpaceBox.MaxCorner();
    glm::vec3 minCorner = modelSpaceBox.MinCorner();
    for (int i = 0; i < 8; ++i) {
        std::bitset<3> bits(i);
        glm::vec3 corner;
        corner.x = bits.test(0) ? maxCorner.x : minCorner.x;
        corner.y = bits.test(1) ? maxCorner.y : minCorner.y;
        corner.z = bits.test(2) ? maxCorner.z : minCorner.z;
        corner = Transform::TransformPoint(corner, modelMat);
        mini = glm::min(mini, corner);
        maxi = glm::max(maxi, corner);
    }

    EntityAABB aabb;
    aabb = AABB(mini, maxi);

    aabb.Entity = entity;
    return aabb;
}

boost::optional<EntityAABB> AbsoluteAABBExplosionEffect(EntityWrapper& entity)
{
    boost::optional<EntityAABB> modelBox = EntityAbsoluteAABB(entity, true);
    if (!modelBox) {
        return boost::none;
    }
    bool isRandom = (bool)entity["ExplosionEffect"]["Randomness"];
    float random = isRandom ? (float)(double)entity["ExplosionEffect"]["RandomnessScalar"] : 0;
    glm::vec3 origin = (glm::vec3)entity["ExplosionEffect"]["ExplosionOrigin"];
    glm::vec3 randomVel = (glm::vec3)entity["ExplosionEffect"]["Velocity"];
    randomVel *= (random + 1);
    float endVelocity = randomVel.y;
    if ((bool)entity["ExplosionEffect"]["ExponentialAccelaration"]) {
        endVelocity *= endVelocity / 2.f;
    }
    float maxRadius = (float)(double)entity["ExplosionEffect"]["ExplosionDuration"] * endVelocity;
    glm::vec3 size;
    AABB explosionBox(origin - (size / 2.f), origin + (size / 2.f));

    glm::vec3 mini = glm::min(explosionBox.MinCorner(), (*modelBox).MinCorner());
    glm::vec3 maxi = glm::max(explosionBox.MaxCorner(), (*modelBox).MaxCorner());
    EntityAABB aabb = AABB(mini, maxi);
    aabb.Entity = entity;
    return aabb;
}

boost::optional<EntityAABB> EntityFirstHitByRay(const Ray& ray, std::vector<EntityAABB> entitiesPotentiallyHitSorted, float& outDistance, glm::vec3& outIntersectPos)
{
    for (EntityAABB& entityBox : entitiesPotentiallyHitSorted) {
        if (!entityBox.Entity.HasComponent("Model")) {
            continue;
        }
        auto& cModel = entityBox.Entity["Model"];
        std::string res = cModel["Resource"];
        if (res.empty() || (bool)cModel["Transparent"] || !((bool)cModel["Visible"])) {
            continue;
        }
        Model* model;
        try {
            model = ResourceManager::Load<::Model, true>(res);
        } catch (const std::exception&) {
            continue;
        }
        float u, v;
        if (RayVsModel(ray, model->Vertices(), model->m_RawModel->m_Indices, Transform::ModelMatrix(entityBox.Entity), outDistance, u, v)) {
            outIntersectPos = ray.Origin() + outDistance * ray.Direction();
            return entityBox;
        }
    }
    return boost::none;
}

boost::optional<EntityAABB> EntityFirstHitByRay(const Ray& ray, Octree<EntityAABB>* octree, float& outDistance, glm::vec3& outIntersectPos)
{
    std::vector<EntityAABB> outObjects;
    octree->ObjectsPossiblyHitByRay(ray, outObjects);
    return Collision::EntityFirstHitByRay(ray, outObjects, outDistance, outIntersectPos);
}

}