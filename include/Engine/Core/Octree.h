#ifndef Octree_h__
#define Octree_h__

#include <type_traits>

#include "../Common.h"
#include "AABB.h"
#include "Frustum.h"
#include "Ray.h"

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
    //Get the objects that are inside the frustum, the objects are put in outObjects.
    void ObjectsInFrustum(const Frustum& frustum, std::vector<T>& outObjects);
    //Get objects, which AABB the input ray intersects, the objects are put in outObjects.
    void ObjectsPossiblyHitByRay(const Ray& ray, std::vector<T>& outObjects);
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
    template<typename T>
    void ObjectsPossiblyHitByRay(const Ray& ray, std::vector<T>& outObjects) const;
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

//To be able to sort child nodes and contained objects based on distance to ray origin.
struct RaySorterInfo
{
    int Index;
    float Distance;
};

bool isFirstLower(const RaySorterInfo& first, const RaySorterInfo& second);

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
void Octree<T>::ObjectsInFrustum(const Frustum& frustum, std::vector<T>& outObjects)
{
    falsifyObjectChecks();
    m_Root->ObjectsInFrustum(frustum, outObjects, false);
}

template<typename T>
void Octree<T>::ObjectsPossiblyHitByRay(const Ray& ray, std::vector<T>& outObjects)
{
    falsifyObjectChecks();
    m_Root->ObjectsPossiblyHitByRay(ray, outObjects);
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
            Frustum::Output out = Frustum::Output::Inside;
            if (!takeAllDontTest) {
                out = frustum.VsAABB(c->m_Box);
                if (out == Frustum::Output::Outside) {
                    continue;
                }
            }
            c->ObjectsInFrustum(frustum, outObjects, out == Frustum::Output::Inside);
        }
    } else {
        size_t startIndex = outObjects.size();
        int numDuplicates = 0;
        outObjects.resize(outObjects.size() + m_StaticObjIndices.size() + m_DynamicObjIndices.size());
        for (size_t i = 0; i < m_StaticObjIndices.size(); ++i) {
            ContainedObject& obj = m_StaticObjectsRef[m_StaticObjIndices[i]];
            if (obj.Checked || frustum.VsAABB(*obj.Box) == Frustum::Output::Outside) {
                ++numDuplicates;
            } else {
                obj.Checked = true;
                outObjects[startIndex + i - numDuplicates] = *static_cast<T*>(obj.Box.get());
            }
        }
        for (size_t i = 0; i < m_DynamicObjIndices.size(); ++i) {
            ContainedObject& obj = m_DynamicObjectsRef[m_DynamicObjIndices[i]];
            if (obj.Checked || frustum.VsAABB(*obj.Box) == Frustum::Output::Outside) {
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
void OctSpace::Child::ObjectsPossiblyHitByRay(const Ray& ray, std::vector<T>& outObjects) const
{
    //If the node AABB is missed, everything it contains is missed.
    if (Collision::RayAABBIntr(ray, m_Box)) {
        //If the ray shoots the tree, and it is a parent.
        if (hasChildren()) {
            //Sort children according to their distance from the ray origin.
            std::vector<RaySorterInfo> childInfos;
            childInfos.resize(8);
            for (int i = 0; i < 8; ++i) {
                childInfos[i] = { i, glm::distance(ray.Origin(), m_Children[i]->m_Box.Origin()) };
            }
            std::sort(childInfos.begin(), childInfos.end(), isFirstLower);
            //Loop through the children, starting with the one closest to the ray origin. I.e the first to be hit.
            for (const RaySorterInfo& info : childInfos) {
                m_Children[info.Index]->ObjectsPossiblyHitByRay(ray, outObjects);
            }
        } else {
            //Check against boxes in the node.
            bool intersected = false;
            float dist;
            //Sort all contained objects according to the distance from the ray origin to 
            //the intersection, if they are intersecting.
            std::vector<RaySorterInfo> objectHitInfos;
            objectHitInfos.reserve(m_StaticObjIndices.size() + m_DynamicObjIndices.size());
            for (int i : m_StaticObjIndices) {
                //If we haven't tested against this object before, and the ray hits.
                if (!m_StaticObjectsRef[i].Checked &&
                        Collision::RayVsAABB(ray, *m_StaticObjectsRef[i].Box, dist)) {
                    objectHitInfos.push_back({ i, dist });
                }
                m_StaticObjectsRef[i].Checked = true;
            }
            for (int i : m_DynamicObjIndices) {
                //If we haven't tested against this object before, and the ray hits.
                if (!m_DynamicObjectsRef[i].Checked &&
                        Collision::RayVsAABB(ray, *m_DynamicObjectsRef[i].Box, dist)) {
                    objectHitInfos.push_back({ i + (int)m_StaticObjIndices.size(), dist });
                }
                m_DynamicObjectsRef[i].Checked = true;
            }
            std::sort(objectHitInfos.begin(), objectHitInfos.end(), isFirstLower);
            int startSize = (int)outObjects.size();
            outObjects.resize(startSize + objectHitInfos.size());
            for (int i = 0; i < objectHitInfos.size(); ++i) {
                outObjects[startSize + i] = (objectHitInfos[i].Index < m_StaticObjIndices.size()) ?
                    *static_cast<T*>(m_StaticObjectsRef[objectHitInfos[i].Index].Box.get()) :
                    *static_cast<T*>(m_DynamicObjectsRef[objectHitInfos[i].Index - m_StaticObjIndices.size()].Box.get());
            }
        }
    }
}


#endif