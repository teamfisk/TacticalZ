#include <vector>
#include <algorithm>
#include <bitset>

#include "Core/Octree.h"
#include "Collision/Collision.h"

namespace
{
//To be able to sort nodes based on distance to ray origin.
struct ChildInfo
{
    int Index;
    float Distance;
};

bool isFirstLower(const ChildInfo& first, const ChildInfo& second)
{
    return first.Distance < second.Distance;
}

}

namespace OctSpace
{

Child::Child(const AABB& octTreeBounds,
    int subDivisions,
    std::vector<ContainedObject>& staticObjects,
    std::vector<ContainedObject>& dynamicObjects)
    : m_Box(octTreeBounds)
    , m_StaticObjectsRef(staticObjects)
    , m_DynamicObjectsRef(dynamicObjects)
{
    if (subDivisions == 0) {
        for (Child*& c : m_Children) {
            c = nullptr;
        }
    } else {
        --subDivisions;
        for (int i = 0; i < 8; ++i) {
            glm::vec3 minPos, maxPos;
            const glm::vec3& parentMin = m_Box.MinCorner();
            const glm::vec3& parentMax = m_Box.MaxCorner();
            const glm::vec3& parentCenter = m_Box.Origin();
            std::bitset<3> bits(i);
            //If child is 4,5,6,7.
            if (bits.test(2)) {
                minPos.x = parentCenter.x;
                maxPos.x = parentMax.x;
            } else {
                minPos.x = parentMin.x;
                maxPos.x = parentCenter.x;
            }

            //If child is 2,3,6,7
            if (bits.test(1)) {
                minPos.y = parentCenter.y;
                maxPos.y = parentMax.y;
            } else {
                minPos.y = parentMin.y;
                maxPos.y = parentCenter.y;
            }
            //If child is 1,3,5,7
            if (bits.test(0)) {
                minPos.z = parentCenter.z;
                maxPos.z = parentMax.z;
            } else {
                minPos.z = parentMin.z;
                maxPos.z = parentCenter.z;
            }
            m_Children[i] = new Child(AABB(minPos, maxPos), subDivisions, m_StaticObjectsRef, m_DynamicObjectsRef);
        }
    }
}

Child::~Child()
{
    for (Child*& c : m_Children) {
        if (c != nullptr) {
            delete c;
            c = nullptr;
        }
    }
}

bool Child::BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const
{
    if (hasChildren()) {
        for (int i : childIndicesContainingBox(boxToTest)) {
            if (m_Children[i]->BoxCollides(boxToTest, outBoxIntersected))
                return true;
        }
    } else {
        for (int i : m_StaticObjIndices) {
            if (!m_StaticObjectsRef[i].Checked) {
                const AABB& objBox = *m_StaticObjectsRef[i].Box;
                if (Collision::AABBVsAABB(boxToTest, objBox)) {
                    outBoxIntersected = objBox;
                    return true;
                }
                m_StaticObjectsRef[i].Checked = true;
            }
        }
        for (int i : m_DynamicObjIndices) {
            if (!m_DynamicObjectsRef[i].Checked) {
                const AABB& objBox = *m_DynamicObjectsRef[i].Box;
                if (!Collision::IsSameBoxProbably(boxToTest, objBox) &&
                    Collision::AABBVsAABB(boxToTest, objBox)) {
                    outBoxIntersected = objBox;
                    return true;
                }
                m_DynamicObjectsRef[i].Checked = true;
            }
        }
    }
    return false;
}

bool Child::RayCollides(const Ray& ray, OctSpace::Output& data) const
{
    //If the node AABB is missed, everything it contains is missed.
    if (Collision::RayAABBIntr(ray, m_Box)) {
        //If the ray shoots the tree, and it is a parent to 8 children :o
        if (hasChildren()) {
            //Sort children according to their distance from the ray origin.
            std::vector<ChildInfo> childInfos;
            childInfos.reserve(8);
            for (int i = 0; i < 8; ++i) {
                childInfos.push_back({ i, glm::distance(ray.Origin(), m_Children[i]->m_Box.Origin()) });
            }
            std::sort(childInfos.begin(), childInfos.end(), isFirstLower);
            //Loop through the children, starting with the one closest to the ray origin. I.e the first to be hit.
            for (const ChildInfo& info : childInfos) {
                if (m_Children[info.Index]->RayCollides(ray, data)) {
                    return true;
                }
            }
        } else {
            //Check against boxes in the node.
            float minDist = INFINITY;
            bool intersected = false;
            for (int i : m_StaticObjIndices) {
                float dist;
                //If we haven't tested against this object before, and the ray hits.
                if (!m_StaticObjectsRef[i].Checked &&
                    Collision::RayVsAABB(ray, *m_StaticObjectsRef[i].Box, dist)) {
                    minDist = std::min(dist, minDist);
                    intersected = true;
                }
                m_StaticObjectsRef[i].Checked = true;
            }
            for (int i : m_DynamicObjIndices) {
                float dist;
                //If we haven't tested against this object before, and the ray hits.
                if (!m_DynamicObjectsRef[i].Checked &&
                    Collision::RayVsAABB(ray, *m_DynamicObjectsRef[i].Box, dist)) {
                    minDist = std::min(dist, minDist);
                    intersected = true;
                }
                m_DynamicObjectsRef[i].Checked = true;
            }

            data.CollideDistance = minDist;
            return intersected;
        }
    }
    return false;
}


void Child::AddDynamicObject(const AABB& box)
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->AddDynamicObject(box);
        }
    } else {
        //Since it hasn't been added yet to the real object list, the index is after the last =size.
        m_DynamicObjIndices.push_back((int)m_DynamicObjectsRef.size());
    }
}

void Child::AddStaticObject(const AABB& box)
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->AddStaticObject(box);
        }
    } else {
        //Since it hasn't been added yet to the real object list, the index is after the last =size.
        m_StaticObjIndices.push_back((int)m_StaticObjectsRef.size());
    }
}

void Child::ClearObjects()
{
    if (hasChildren()) {
        for (Child*& c : m_Children) {
            c->ClearObjects();
        }
    } else {
        m_DynamicObjIndices.clear();
        m_StaticObjIndices.clear();
    }
}

void Child::ClearDynamicObjects()
{
    if (hasChildren()) {
        for (Child*& c : m_Children) {
            c->ClearDynamicObjects();
        }
    } else {
        m_DynamicObjIndices.clear();
    }
}

//:     3        7 
//:
//:        2         6 
//:                           |
//:     1        5          \ y
//:                          z 
//:        0         4        0  x-->
//
// child: 0 1 2 3 4 5 6 7
//    x : - - - - + + + +
//    y : - - + + - - + +
//    z : - + - + - + - +
int Child::childIndexContainingPoint(const glm::vec3& point) const
{
    const glm::vec3& c = m_Box.Origin();
    return (1 << 2) * (point.x >= c.x) | (1 << 1) * (point.y >= c.y) | (point.z >= c.z);
}

std::vector<int> Child::childIndicesContainingBox(const AABB& box) const
{
    int minInd = childIndexContainingPoint(box.MinCorner());
    int maxInd = childIndexContainingPoint(box.MaxCorner());
    //Because of the predictable ordering of the child indices, 
    //the number of bits set when xor:ing the indices will determine the number of children containing the box.
    std::bitset<3> bits(minInd ^ maxInd);
    switch (bits.count()) {
        //Box contained completely in one child.
    case 0:
        return{ minInd };
        //Two children.
    case 1:
        return{ minInd, maxInd };
        //Four children.
    case 2:
    {
        std::vector<int> ret;
        //Bit-hax to calculate the correct 4 children containing the box.
        //This works because of the childrens index determine what part of 
        //the dimensions they are responsible for (which octant).
        bits.flip();
        //At this point the bits necessarily have exactly one bit set.
        //Check the same bit in the minInd as the one set in bits.
        int setOrUnset = (bits.to_ulong() & minInd);
        for (int c = 0; c < 8; ++c) {
            //Check the same bit in the child index as the one set in bits.
            //Enter here if both c and minInd have the bit set, or if neither have it set.
            //I.e, if they are on the same side (+ or -) in the dimension marked by the bit in bits.
            if (!((bits.to_ulong() & c) ^ setOrUnset)) {
                ret.push_back(c);
            }
        }
        return ret;
    }
    case 3:                         //Eight children.
        return{ 0,1,2,3,4,5,6,7 };
    default:
        return std::vector<int>();
    }
}

}