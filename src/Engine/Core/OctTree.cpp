#include <vector>
#include <algorithm>
#include <bitset>

#include "Core/OctTree.h"
#include "Core/Collision.h"
#include "Core/World.h"
#include "Rendering/Camera.h"

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
    , m_UpdatedOnce(false)
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
            const glm::vec3& parentCenter = m_Box.Center();
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

void OctTree::Update(float dt, World* world, Camera* cam)
{
    AABB aabb;
    for (ComponentWrapper& c : world->GetComponents("Collision")) {
        aabb.CreateFromCenter(c["BoxCenter"], c["BoxSize"]);
        AddStaticObject(aabb);
    }
    const glm::vec4 redCol = glm::vec4(1, 0.2f, 0, 1);
    const glm::vec4 greenCol = glm::vec4(0.1f, 1.0f, 0.25f, 1);
    const glm::vec3 boxSize = 0.1f*glm::vec3(1.0f, 1.0f, 1.0f);

    if (!m_UpdatedOnce) {
        m_BoxID = world->CreateEntity();
        ComponentWrapper transform = world->AttachComponent(m_BoxID, "Transform");
        transform["Scale"] = boxSize;
        ComponentWrapper model = world->AttachComponent(m_BoxID, "Model");
        model["Resource"] = "Models/Core/UnitBox.obj";
        m_UpdatedOnce = true;
    }

    AABB box;
    auto boxPos = cam->Position() + 1.2f*cam->Forward();
    box.CreateFromCenter(boxPos, boxSize);
    ComponentWrapper transform = world->GetComponent(m_BoxID, "Transform");
    transform["Position"] = boxPos;
    ComponentWrapper model = world->GetComponent(m_BoxID, "Model");
    //if (BoxCollides(box, AABB())) {
    if (Collision::AABBVsAABB(box, aabb)) {
        cam->SetPosition(m_PrevPos);
        cam->SetOrientation(m_PrevOri);
        model["Color"] = greenCol;
    } else {
        model["Color"] = redCol;
    }

    m_PrevPos = cam->Position();
    m_PrevOri = cam->Orientation();
    ClearObjects();
}

bool OctTree::BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const
{
    if (hasChildren()) {
        for (int i : childIndicesContainingBox(boxToTest)) {
            if (m_Children[i]->BoxCollides(boxToTest, outBoxIntersected))
                return true;
        }
    } else {
        std::vector<std::vector<AABB>> objVectors = {
            m_StaticObjects,
            m_DynamicObjects
        };
        for (const auto& objVector : objVectors) {
            for (const auto& obj : objVector) {
                if (Collision::AABBVsAABB(boxToTest, obj)) {
                    outBoxIntersected = obj;
                    return true;
                }
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
        } else {
            //Check against boxes in the node.
            float minDist = INFINITY;
            bool intersected = false;
            std::vector<std::vector<AABB>> objVectors = {
                m_StaticObjects,
                m_DynamicObjects
            };
            for (const auto& objVector : objVectors) {
                for (const auto& obj : objVector) {
                    float dist;
                    if (Collision::RayVsAABB(ray, obj, dist)) {
                        minDist = std::min(dist, minDist);
                        intersected = true;
                    }
                }
            }

            data.CollideDistance = minDist;
            return intersected;
        }
    }
    return false;
}


void OctTree::AddDynamicObject(const AABB& box)
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->AddDynamicObject(box);
        }
    } else {
        m_DynamicObjects.push_back(box);
    }
}

void OctTree::AddStaticObject(const AABB& box)
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->AddStaticObject(box);
        }
    } else {
        m_StaticObjects.push_back(box);
    }
}

void OctTree::BoxesInSameRegion(const AABB& box, std::vector<AABB>& outBoxes) const
{
    if (hasChildren()) {
        for (auto i : childIndicesContainingBox(box)) {
            m_Children[i]->BoxesInSameRegion(box, outBoxes);
        }
    } else {
        outBoxes.insert(outBoxes.end(), m_StaticObjects.begin(), m_StaticObjects.end());
        outBoxes.insert(outBoxes.end(), m_DynamicObjects.begin(), m_DynamicObjects.end());
    }
}

void OctTree::ClearObjects()
{
    if (hasChildren()) {
        for (OctTree*& c : m_Children) {
            c->ClearObjects();
        }
    } else {
        m_DynamicObjects.clear();
        m_StaticObjects.clear();
    }
}

void OctTree::ClearDynamicObjects()
{
    if (hasChildren()) {
        for (OctTree*& c : m_Children) {
            c->ClearObjects();
        }
    } else {
        m_DynamicObjects.clear();
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
        //Bit-hax to calculate the correct 4 children containing the box.
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