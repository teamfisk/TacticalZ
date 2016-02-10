#ifndef Octree_h__
#define Octree_h__

#include <type_traits>

#include "../Common.h"
#include "AABB.h"
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

    bool hasChildren() const;
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

#endif