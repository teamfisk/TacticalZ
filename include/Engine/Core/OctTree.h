#ifndef OctTree_h__
#define OctTree_h__

#include "Core/AABB.h"

struct Ray;
class World;
class Camera;

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

    //We should only ever need one OctTree in the game, and it should not need to be copied.
    //Define these if the OctTree suddenly needs to be copied, think of the children OctTree* ptrs. 
    OctTree(const OctTree& other) = delete;
    OctTree(const OctTree&& other) = delete;
    OctTree& operator= (const OctTree& other) = delete;

    void AddDynamicObject(const AABB& box);
    void AddStaticObject(const AABB& box);

    void BoxesInSameRegion(const AABB& box, std::vector<AABB>& outBoxes) const;

    void ClearObjects();
    void ClearDynamicObjects();

    //Collision test function. WTODO: Probably remove or relocate elsewhere, Collision system?
    void Update(float dt, World* world, Camera* cam);
    //Returns true if the ray collides with something in the tree. Result is written to [data].
    bool RayCollides(const Ray& ray, Output& data) const;
    //Returns true if the box collides with something in the tree. 
    //On collision with a box, that box is written to [outBoxIntersected].
    //Note: More efficient than calling BoxesInSameRegion from outside and testing there.
    bool BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const;

private:
    OctTree* m_Children[8];
    //WTODO: Do -derived class from AABB- struct containing AABB, with a bool Tested, falsify at 
    //start of Collision test, set on check, don't check if set already. Solves duplicate boxes in tree. 
    //Store indices in the struct, pointing to grand ancestor list of boxes, need the same AABB not copies to save Tested.
    std::vector<AABB> m_StaticObjects;
    std::vector<AABB> m_DynamicObjects;
    AABB m_Box;
                                                                                                                   
    bool m_UpdatedOnce;
    unsigned int m_BoxID;
    glm::vec3 m_PrevPos;
    glm::quat m_PrevOri;

    inline bool hasChildren() const;
    int childIndexContainingPoint(const glm::vec3& point) const;
    std::vector<int> childIndicesContainingBox(const AABB& box) const;
};

#endif