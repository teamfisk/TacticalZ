#ifndef Octree_h__
#define Octree_h__

#include "Core/AABB.h"

class Ray;

class Octree
{
public:
    struct Output
    {
        float CollideDistance;
    };

    Octree();
    ~Octree();
    //For the root Octree, [octreeBounds] should be a box containing the entire level.
    Octree(const AABB& octreeBounds, int subDivisions);

    //We cannot copy the Octree as of now, because of the recursive dynamic allocation.
    //Define these if the Octree suddenly needs to be copied, think of the children Child* ptrs. 
    Octree(const Octree& other) = delete;
    Octree(const Octree&& other) = delete;
    Octree& operator= (const Octree& other) = delete;
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

    //Returns true if the ray collides with something in the tree. Result is written to [data].
    bool RayCollides(const Ray& ray, Output& data);
    //Returns true if the box collides with something in the tree. 
    //On collision with a box, that box is written to [outBoxIntersected].
    //Note: More efficient than calling BoxesInSameRegion from outside and testing there.
    bool BoxCollides(const AABB& boxToTest, AABB& outBoxIntersected);

private:
    struct Child;    //Fwd declaration;
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
    Child* m_Root;
    std::vector<ContainedObject> m_StaticObjects;
    std::vector<ContainedObject> m_DynamicObjects;

    bool m_UpdatedOnce;
    unsigned int m_BoxID;
    glm::vec3 m_PrevPos;
    glm::quat m_PrevOri;

    void falsifyObjectChecks();

    struct Child
    {
        ~Child();
        Child(const AABB& octTreeBounds, 
            int subDivisions, 
            std::vector<Octree::ContainedObject>& staticObjects, 
            std::vector<Octree::ContainedObject>& dynamicObjects);
        Child(const Child& other) = delete;
        Child(const Child&& other) = delete;
        Child& operator= (const Child& other) = delete;
        void AddDynamicObject(const AABB& box);
        void AddStaticObject(const AABB& box);
        void BoxesInSameRegion(const AABB& box, std::vector<AABB>& outBoxes) const;
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
        std::vector<Octree::ContainedObject>& m_StaticObjectsRef;
        std::vector<Octree::ContainedObject>& m_DynamicObjectsRef;

        inline bool hasChildren() const;
        int childIndexContainingPoint(const glm::vec3& point) const;
        std::vector<int> childIndicesContainingBox(const AABB& box) const;
    };
};


#endif