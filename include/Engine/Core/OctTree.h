#ifndef OctTree_h__
#define OctTree_h__

#include "Core/Collision.h"

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

private:
    OctTree* m_Children[8];
    std::vector<AABB> m_ContainingBoxes;
    //TODO: Do derived class from AABB with a bool Tested, falsify at 
    //start of Collision test, set on check, don't check if set already. Solves duplicate boxes in tree.
    AABB m_Box;

    bool rayCollides(const Ray& ray, Output& data, const OctTree* const tree) const;
    inline bool hasChildren() const;
    int childIndexContainingPoint(const glm::vec3& point) const;
};

#endif