#include <vector>
#include <algorithm>
#include <bitset>

#include "Core/OctTree.h"
#include "Core/Collision.h"

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

OctTree::OctTree()
    : OctTree(AABB(), 0)
{}

OctTree::OctTree(const AABB& octTreeBounds, int subDivisions)
    : m_Box(octTreeBounds)
{
    if (subDivisions == 0) {
        for (OctTree*& c : m_Children) {
            c = nullptr;
        }
    }
    else {
        --subDivisions;
        const glm::vec3& parentMin = m_Box.MinCorner();
        const glm::vec3& parentMax = m_Box.MaxCorner();
        const glm::vec3& parentCenter = m_Box.Center();
        for (int i = 0; i < 8; ++i) {
            glm::vec3 minPos, maxPos;
            std::bitset<3> bits(i);
            //If child is 4,5,6,7.
            if (bits.test(2)) {
                minPos.x = parentCenter.x;
                maxPos.x = parentMax.x;
            }
            else {
                minPos.x = parentMin.x;
                maxPos.x = parentCenter.x;
            }

            //If child is 2,3,6,7
            if (bits.test(1)) {
                minPos.y = parentCenter.y;
                maxPos.y = parentMax.y;
            }
            else {
                minPos.y = parentMin.y;
                maxPos.y = parentCenter.y;
            }
            //If child is 1,3,5,7
            if (bits.test(0)) {
                minPos.z = parentCenter.z;
                maxPos.z = parentMax.z;
            }
            else {
                minPos.z = parentMin.z;
                maxPos.z = parentCenter.z;
            }
            m_Children[i] = new OctTree(AABB(minPos, maxPos), subDivisions);
        }
    }
}

OctTree::~OctTree()
{
    for (OctTree*& c : m_Children) {
        if (c != nullptr) {
            //recursively delete (this calls the deconstructor again)
            delete c;
            c = nullptr;
        }
    }
}

bool OctTree::BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const
{
    if (hasChildren()) {
        for (int i : childIndicesContainingBox(boxToTest)) {
            if (m_Children[i]->BoxCollides(boxToTest, outBoxIntersected))
                return true;
        }
    } else {
        for (const auto& objBox : m_ContainingBoxes) {
            if (Collision::AABBVsAABB(boxToTest, objBox)) {
                outBoxIntersected = objBox;
                return true;
            }
        }
    }
    return false;
}

bool OctTree::RayCollides(const Ray& ray, Output& data) const
{
    //If the node AABB is missed, everything it contains is missed.
    if (Collision::RayAABBIntr(ray, m_Box)) {
        //If the ray shoots the tree, and it is a parent to 8 children :o
        if (hasChildren()) {
            //Sort children according to their distance from the ray origin.
            std::vector<ChildInfo> childInfos;
            childInfos.reserve(8);
            for (int i = 0; i < 8; ++i) {
                childInfos.push_back({ i, glm::distance(ray.Origin, m_Children[i]->m_Box.Center()) });
            }
            std::sort(childInfos.begin(), childInfos.end(), isFirstLower);
            //Loop through the children, starting with the one closest to the ray origin. I.e the first to be hit.
            for (const ChildInfo& info : childInfos) {
                if (m_Children[info.Index]->RayCollides(ray, data)) {
                    return true;
                }
            }
        }
        else {
            //Check against boxes in the node.
            float minDist = INFINITY;
            bool intersected = false;
            for (const auto& objBox : m_ContainingBoxes) {
                float dist;
                if (Collision::RayVsAABB(ray, objBox, dist)) {
                    minDist = std::min(dist, minDist);
                    intersected = true;
                }
            }
            data.CollideDistance = minDist;
            return intersected;
        }
    }
    return false;
}

void OctTree::AddBox(const AABB& box)
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->AddBox(box);
        }
    }
    else {
        m_ContainingBoxes.push_back(box);
    }
}

//remove the content (boxes) in the tree, but dont rememove the tree-structure
//TODO: Only clear dynamic boxes, AddDynamic, AddStatic
void OctTree::ClearBoxes()
{
    if (hasChildren()) {
        for (OctTree*& c : m_Children) {
            c->ClearBoxes();
        }
    }
    else {
        m_ContainingBoxes.clear();
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
int OctTree::childIndexContainingPoint(const glm::vec3& point) const
{
    const glm::vec3& c = m_Box.Center();
    return (1 << 2) * (point.x >= c.x) | (1 << 1) * (point.y >= c.y) | (point.z >= c.z);
}

std::vector<int> OctTree::childIndicesContainingBox(const AABB& box) const
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
        //Bit-hax to calculate the right 4 cildren containing the box.
        //This works because of the childrens index determine what part of 
        //the dimensions they are responsible for (which octant).
        bits.flip();
        //At this point the bits necessarily have exactly one bit set.
        for (int c = 0; c < 8; ++c) {
            //If the child index have the same bit set as the bits, add box to it.
            if (bits.to_ulong() & c) {
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

inline bool OctTree::hasChildren() const
{
    return m_Children[0] != nullptr;
}