#include <vector>
#include <algorithm>
#include <bitset>

#include "Core/OctTree.h"

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
    } else {
        --subDivisions;
        for (int i = 0; i < 8; ++i) {
            glm::vec3 minPos, maxPos;
            const glm::vec3& parentMin = m_Box.MinCorner();
            const glm::vec3& parentMax = m_Box.MaxCorner();
            const glm::vec3& parentCenter = m_Box.MaxCorner();
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
            m_Children[i] = new OctTree(AABB(minPos, maxPos), subDivisions);
        }
    }
}

OctTree::~OctTree()
{
    for (OctTree*& c : m_Children) {
        if (c != nullptr) {
            delete c;
            c = nullptr;
        }
    }
}


bool OctTree::RayCollides(const Ray& ray, Output& data) const
{
    data.CollideDistance = -1;
    return rayCollides(ray, data, this);
}

//Currently all Nodes must have exactly 0 or 8 children, and objectdata should only exist in the last bottom nodes.
bool OctTree::rayCollides(const Ray& ray, Output& data, const OctTree* const tree) const
{
    //If the node AABB is missed, everything it contains is missed.
    if (Collision::RayAABBIntr(ray, tree->m_Box)) {
        //If the ray shoots the tree, and it is a parent to 8 children :o
        if (tree->hasChildren()) {
            //Sort children according to their distance from the ray origin.
            std::vector<ChildInfo> childInfos;
            childInfos.reserve(8);
            for (int i = 0; i < 8; ++i) {
                childInfos.push_back({ i, glm::distance(ray.Origin, tree->m_Children[i]->m_Box.Center()) });
            }
            std::sort(childInfos.begin(), childInfos.end(), isFirstLower);
            //Loop through the children, starting with the one closest to the ray origin. I.e the first to be hit.
            for (const ChildInfo& info : childInfos) {
                if (rayCollides(ray, data, tree->m_Children[info.Index])) {
                    return true;
                }
            }
        } else {
            ////TODO: Check against objects in the node.
            //float minDist = INFINITY;
            //for (const auto& obj : m_ObjectsInBox) {
            //    float dist = Collide(ray, obj);
            //    minDist = min(dist, minDist);
            //}
            //data.CollideDistance = minDist;
            //if minDist != Collide()'s non-collide value: return false;
            return true;
        }
    }
    return false;
}

void OctTree::AddBox(const AABB& box)
{
    if (hasChildren()) {
        int minInd = childIndexContainingPoint(box.MinCorner());
        int maxInd = childIndexContainingPoint(box.MaxCorner());
        std::bitset<3> bits(minInd ^ maxInd);
        switch (bits.count()) {
        case 0:                         //Box contained completely in one child.
            m_Children[minInd]->AddBox(box);
            break;
        case 1:                         //Two children.
            m_Children[minInd]->AddBox(box);
            m_Children[maxInd]->AddBox(box);
            break;
        case 2:                         //Four children.
            bits.flip();
            for (int c = 0; c < 8; ++c) {
                if ((bits & std::bitset<3>(c))[0]) {
                    m_Children[c]->AddBox(box);
                }
            }
            break;
        case 3:                         //Eight children.
            for (OctTree*& c : m_Children) {
                c->AddBox(box);
            }
            break;
        default:
            break;
        }
    } else {
        m_ContainingBoxes.push_back(box);
    }
}

void OctTree::ClearBoxes()
{
    if (hasChildren()) {
        for (OctTree*& c : m_Children) {
            c->ClearBoxes();
        }
    } else {
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

inline bool OctTree::hasChildren() const
{
    return m_Children[0] != nullptr;
}