#ifndef Octree_h__
#define Octree_h__

#include <type_traits>
#include <bitset>

#include "../Common.h"
#include "AABB.h"

//Fwd declarations.
class Ray;

namespace OctSpace
{
struct Output;
struct ContainedObject;
struct Child;
}

//T needs to be AABB, or inherit from AABB.
//T also needs to have a default constructor.
template<typename T>
class Octree
{
public:
    Octree() = delete;
    ~Octree();
    //For the root Octree, [octreeBounds] should be a box containing the entire level.
    Octree(const AABB& octreeBounds, int subDivisions);

    //We cannot copy the Octree as of now, because of the recursive dynamic allocation.
    //Define these if the Octree suddenly needs to be copied, think of the children Child* ptrs. 
    Octree(const Octree& other) = delete;
    Octree(const Octree&& other) = delete;
    Octree& operator= (const Octree& other) = delete;
    //Add a dynamic object (one that moves around) into the tree.
    void AddDynamicObject(const T& object);
    //Add a static object (that does not move) into the tree.
    void AddStaticObject(const T& object);
    //Get the objects that are in the same area as the input [box], the objects are put in [outObjects].
    //The type Box must be AABB, or inherit from AABB.
    template<typename Box>
    void ObjectsInSameRegion(const Box& box, std::vector<T>& outObjects);
    //Get the objects that are inside the frustum defined by the viewProjection matrix, the objects are put in outObjects.
    void ObjectsInFrustum(const glm::mat4x4& viewProj, std::vector<T>& outObjects);
    //Empty the tree of all objects, static and dynamic.
    void ClearObjects();
    //Empty the tree of all dynamic objects. Static objects remain in the tree.
    void ClearDynamicObjects();

    //Returns true if the ray collides with something in the tree. Result is written to [data].
    bool RayCollides(const Ray& ray, OctSpace::Output& data);
    //Returns true if the box collides with something in the tree. 
    //On collision with a box, that box is written to [outBoxIntersected].
    //Note: More efficient than calling ObjectsInSameRegion from outside and testing there.
    bool BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected);

private:
    OctSpace::Child* m_Root;
    std::vector<OctSpace::ContainedObject> m_StaticObjects;
    std::vector<OctSpace::ContainedObject> m_DynamicObjects;

    void falsifyObjectChecks();
};

namespace OctSpace
{

struct Output
{
    float CollideDistance;
};

//Contains points P in: dot(normal, P) + d = 0
struct Plane
{
    glm::vec3 normal;
    float distance;
};

//A frustum defined by 6 planes.
struct Frustum
{
    enum Output
    {
        Inside,
        Outside,
        Intersects
    };
    Plane planes[6];

    Output VsAABB(const AABB& box) const
    {
        const glm::vec3& maxCorner = box.MaxCorner();
        const glm::vec3& minCorner = box.MinCorner();
        bool completelyInside = true;
        for (const Plane& p : planes) {
            bool anyWasInside = false;
            bool anyWasOutside = false;
            //If points are on both sides of the plane, we can stop.
            for (int i = 0; i < 8 && (!anyWasInside || !anyWasOutside); ++i) {
                std::bitset<3> bits(i);
                glm::vec3 corner;
                corner.x = bits.test(0) ? maxCorner.x : minCorner.x;
                corner.y = bits.test(1) ? maxCorner.y : minCorner.y;
                corner.z = bits.test(2) ? maxCorner.z : minCorner.z;
                if (glm::dot(p.normal, corner) > p.distance) {
                    anyWasInside = true;
                } else {
                    anyWasOutside = true;
                }
            }
            if (!anyWasInside) {
                return Outside;
            }
            if (anyWasOutside) {
                completelyInside = false;
            }
        }
        return completelyInside ? Inside : Intersects;
    }
};

struct ContainedObject
{
    ContainedObject()
        : Box(nullptr)
        , Checked(false)
    {}
    template<typename BoxlikeObject>
    ContainedObject(const BoxlikeObject& box)
        : Box(new BoxlikeObject(box))
        , Checked(false)
    {}
    std::unique_ptr<AABB> Box;
    bool Checked;
};

struct Child
{
    ~Child();
    Child(const AABB& octTreeBounds,
        int subDivisions,
        std::vector<ContainedObject>& staticObjects,
        std::vector<ContainedObject>& dynamicObjects);
    Child(const Child& other) = delete;
    Child(const Child&& other) = delete;
    Child& operator= (const Child& other) = delete;
    void AddDynamicObject(const AABB& box);
    void AddStaticObject(const AABB& box);
    template<typename T, typename Box>
    void ObjectsInSameRegion(const Box& box, std::vector<T>& outObjects) const;
    template<typename T>
    void ObjectsInFrustum(const Frustum& frustum, std::vector<T>& outObjects, bool takeAllDontTest) const;
    void ClearObjects();
    void ClearDynamicObjects();
    bool RayCollides(const Ray& ray, Output& data) const;
    bool BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const;

    Child* m_Children[8];
    //Indices into the lists in Octree.
    std::vector<int> m_StaticObjIndices;
    std::vector<int> m_DynamicObjIndices;
    AABB m_Box;
    //Reference to the lists in Octree.
    std::vector<ContainedObject>& m_StaticObjectsRef;
    std::vector<ContainedObject>& m_DynamicObjectsRef;

    inline bool hasChildren() const { return m_Children[0] != nullptr; }
    int childIndexContainingPoint(const glm::vec3& point) const;
    std::vector<int> childIndicesContainingBox(const AABB& box) const;
};

}

template<typename T>
Octree<T>::Octree(const AABB& octTreeBounds, int subDivisions)
    : m_Root(new OctSpace::Child(octTreeBounds, subDivisions, m_StaticObjects, m_DynamicObjects))
{
    static_assert(std::is_base_of<AABB, T>::value, "template argument type T in Octree must be a subclass of AABB.");
}

template<typename T>
Octree<T>::~Octree()
{
    delete m_Root;
}

template<typename T>
void Octree<T>::AddDynamicObject(const T& object)
{
    m_Root->AddDynamicObject(object);
    m_DynamicObjects.emplace_back(object);
}

template<typename T>
void Octree<T>::AddStaticObject(const T& object)
{
    m_Root->AddStaticObject(object);
    m_StaticObjects.emplace_back(object);
}

template<typename T>
template<typename Box>
void Octree<T>::ObjectsInSameRegion(const Box& box, std::vector<T>& outObjects)
{
    static_assert(std::is_base_of<AABB, Box>::value, "template argument type Box in Octree<T>::ObjectsInSameRegion must be a subclass of AABB.");
    falsifyObjectChecks();
    m_Root->ObjectsInSameRegion(box, outObjects);
}

template<typename T>
void Octree<T>::ObjectsInFrustum(const glm::mat4x4& viewProj, std::vector<T>& outObjects)
{
    falsifyObjectChecks();
    OctSpace::Frustum frustum;
    for (int i = 0; i < 6; ++i) {
        int sign = 2 * (i % 2) - 1;
        int index = i / 2;
        OctSpace::Plane& plane = frustum.planes[i];
        plane.normal.x = viewProj[0].w + sign * viewProj[0][index];
        plane.normal.y = viewProj[1].w + sign * viewProj[1][index];
        plane.normal.z = viewProj[2].w + sign * viewProj[2][index];
        plane.distance = viewProj[3].w + sign * viewProj[3][index];
        float divByNormalLength = 1.0f / glm::length(plane.normal);
        plane.normal *= divByNormalLength;
        plane.distance *= divByNormalLength;
    }
    m_Root->ObjectsInFrustum(frustum, outObjects, false);
}

template<typename T>
void Octree<T>::ClearObjects()
{
    m_StaticObjects.clear();
    m_DynamicObjects.clear();
    m_Root->ClearObjects();
}

template<typename T>
void Octree<T>::ClearDynamicObjects()
{
    m_DynamicObjects.clear();
    m_Root->ClearDynamicObjects();
}

template<typename T>
bool Octree<T>::RayCollides(const Ray& ray, OctSpace::Output& data)
{
    falsifyObjectChecks();
    data.CollideDistance = -1;
    return m_Root->RayCollides(ray, data);
}

template<typename T>
bool Octree<T>::BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected)
{
    falsifyObjectChecks();
    return m_Root->BoxCollides(boxToTest, outBoxIntersected);
}

template<typename T>
void Octree<T>::falsifyObjectChecks()
{
    for (auto& obj : m_StaticObjects) {
        obj.Checked = false;
    }
    for (auto& obj : m_DynamicObjects) {
        obj.Checked = false;
    }
}

template<typename T, typename Box>
void OctSpace::Child::ObjectsInSameRegion(const Box& box, std::vector<T>& outObjects) const
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->ObjectsInSameRegion(box, outObjects);
        }
    } else {
        size_t startIndex = outObjects.size();
        int numDuplicates = 0;
        outObjects.resize(outObjects.size() + m_StaticObjIndices.size() + m_DynamicObjIndices.size());
        for (size_t i = 0; i < m_StaticObjIndices.size(); ++i) {
            ContainedObject& obj = m_StaticObjectsRef[m_StaticObjIndices[i]];
            if (obj.Checked) {
                ++numDuplicates;
            } else {
                obj.Checked = true;
                outObjects[startIndex + i - numDuplicates] = *static_cast<T*>(obj.Box.get());
            }
        }
        for (size_t i = 0; i < m_DynamicObjIndices.size(); ++i) {
            ContainedObject& obj = m_DynamicObjectsRef[m_DynamicObjIndices[i]];
            if (obj.Checked) {
                ++numDuplicates;
            } else {
                obj.Checked = true;
                outObjects[startIndex + i - numDuplicates] = *static_cast<T*>(obj.Box.get());
            }
        }
        for (size_t i = 0; i < numDuplicates; ++i) {
            outObjects.pop_back();
        }
    }
}

template<typename T>
void OctSpace::Child::ObjectsInFrustum(const Frustum& frustum, std::vector<T>& outObjects, bool takeAllDontTest) const
{
    if (hasChildren()) {
        for (const Child* c : m_Children) {
            Frustum::Output out = Frustum::Inside;
            if (!takeAllDontTest) {
                out = frustum.VsAABB(c->m_Box);
                if (out == Frustum::Outside) {
                    continue;
                }
            }
            c->ObjectsInFrustum(frustum, outObjects, out == Frustum::Inside);
        }
    } else {
        size_t startIndex = outObjects.size();
        int numDuplicates = 0;
        outObjects.resize(outObjects.size() + m_StaticObjIndices.size() + m_DynamicObjIndices.size());
        for (size_t i = 0; i < m_StaticObjIndices.size(); ++i) {
            ContainedObject& obj = m_StaticObjectsRef[m_StaticObjIndices[i]];
            if (obj.Checked || !frustum.VsAABB(obj.Box)) {
                ++numDuplicates;
            } else {
                obj.Checked = true;
                outObjects[startIndex + i - numDuplicates] = *static_cast<T*>(obj.Box.get());
            }
        }
        for (size_t i = 0; i < m_DynamicObjIndices.size(); ++i) {
            ContainedObject& obj = m_DynamicObjectsRef[m_DynamicObjIndices[i]];
            if (obj.Checked || !frustum.VsAABB(obj.Box)) {
                ++numDuplicates;
            } else {
                obj.Checked = true;
                outObjects[startIndex + i - numDuplicates] = *static_cast<T*>(obj.Box.get());
            }
        }
        for (size_t i = 0; i < numDuplicates; ++i) {
            outObjects.pop_back();
        }
    }
}

#endif