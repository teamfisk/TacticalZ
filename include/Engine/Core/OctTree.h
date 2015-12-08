#ifndef OctTree_h__
#define OctTree_h__

#include "Core/AABB.h"

struct Ray;

class OctTree
{
public:
    struct Output
    {
        float CollideDistance;
    }; 
    OctTree();
    ~OctTree();
    //For the root OctTree, [octTreeBounds] should be a box containing the entire level.
    OctTree(const AABB& octTreeBounds, int subDivisions);
    void AddBox(const AABB& box);
    void ClearBoxes();
    //Returns true if the ray collides with something in the tree. Result is written to [data].
    bool RayCollides(const Ray& ray, Output& data) const;
    //Returns true if the box collides with something in the tree. 
    //On collision with a box, that box is written to [outBoxIntersected].
    bool BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const;

private:
    OctTree* m_Children[8];
    //TODO: Do derived class from AABB with a bool Tested, falsify at 
    //start of Collision test, set on check, don't check if set already. Solves duplicate boxes in tree.
    //TODO: Boxes collide with themselves? Fix somehow, maybe float epsilon stuff.
    std::vector<AABB> m_ContainingBoxes;
    AABB m_Box;

    inline bool hasChildren() const;
    int childIndexContainingPoint(const glm::vec3& point) const;
    std::vector<int> childIndicesContainingBox(const AABB& box) const;
};

#endif