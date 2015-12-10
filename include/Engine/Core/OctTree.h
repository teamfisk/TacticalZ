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
    //Add a dynamic object (one that moves around) into the tree.
    void AddDynamicObject(const AABB& box);
    //Add a static object (that does not move) into the tree.
    void AddStaticObject(const AABB& box);
    //Get the boxes that are in the same area as the input [box], the boxes are put in [outBoxes].
    void BoxesInSameRegion(const AABB& box, std::vector<AABB>& outBoxes);
    //Empty the tree of all objects, static and dynamic.
    void ClearObjects();
    //Empty the tree of all dynamic objects. Static objects remain in the tree.
    void ClearDynamicObjects();

    //Collision test function. WTODO: Probably remove or relocate elsewhere, Collision system?
    void Update(float dt, World* world, Camera* cam);
    //Returns true if the ray collides with something in the tree. Result is written to [data].
    bool RayCollides(const Ray& ray, Output& data);
    //Returns true if the box collides with something in the tree. 
    //On collision with a box, that box is written to [outBoxIntersected].
    //Note: More efficient than calling BoxesInSameRegion from outside and testing there.
    bool BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected);

private:
    struct OctChild;    //Fwd declaration;
    struct ContainedObject
    {
        ContainedObject()
            : Box(AABB())
            , Checked(false)
        {}
        ContainedObject(AABB box)
            : Box(box)
            , Checked(false)
        {}
        AABB Box;
        bool Checked;
    };
    OctChild* m_Root;
    std::vector<ContainedObject> m_StaticObjects;
    std::vector<ContainedObject> m_DynamicObjects;

    bool m_UpdatedOnce;
    unsigned int m_BoxID;
    glm::vec3 m_PrevPos;
    glm::quat m_PrevOri;

    void falsifyObjectChecks();

    struct OctChild
    {
        ~OctChild();
        OctChild(const AABB& octTreeBounds, 
            int subDivisions, 
            std::vector<OctTree::ContainedObject>& staticObjects, 
            std::vector<OctTree::ContainedObject>& dynamicObjects);
        OctChild(const OctChild& other) = delete;
        OctChild(const OctChild&& other) = delete;
        OctChild& operator= (const OctChild& other) = delete;
        void AddDynamicObject(const AABB& box);
        void AddStaticObject(const AABB& box);
        void BoxesInSameRegion(const AABB& box, std::vector<AABB>& outBoxes) const;
        void ClearObjects();
        void ClearDynamicObjects();
        bool RayCollides(const Ray& ray, Output& data) const;
        bool BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected) const;

        OctChild* m_Children[8];
        //Indices into the lists in OctTree.
        std::vector<int> m_StaticObjIndices;
        std::vector<int> m_DynamicObjIndices;
        AABB m_Box;
        //Reference to the lists in OctTree.
        std::vector<OctTree::ContainedObject>& m_StaticObjectsRef;
        std::vector<OctTree::ContainedObject>& m_DynamicObjectsRef;

        inline bool hasChildren() const;
        int childIndexContainingPoint(const glm::vec3& point) const;
        std::vector<int> childIndicesContainingBox(const AABB& box) const;
    };
};


#endif