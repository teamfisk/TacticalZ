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

    //Test Getters.
    OctTree** Children() { return m_Children; }
    const AABB& Box() { return m_Box; }
    const std::vector<AABB>& ContainingBoxes() { return m_ContainingBoxes; }

private:
    OctTree* m_Children[8];
    std::vector<AABB> m_ContainingBoxes;
    //TODO: Do derived class from AABB with a bool Tested, falsify at 
    //start of Collision test, set on check, don't check if set already. Solves duplicate boxes in tree.
    AABB m_Box;
    
    bool rayCollides(const Ray& ray, Output& data) const;
    inline bool hasChildren() const;
    int childIndexContainingPoint(const glm::vec3& point) const;
};

#endif