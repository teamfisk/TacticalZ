#ifndef BlendTree_h__
#define BlendTree_h__

#include "Common.h"
#include "../GLM.h"
#include "Skeleton.h"
#include "../Core/EntityWrapper.h"
#include "../Core/World.h"
#include <stack>

class BlendTree
{
public:
    enum class NodeType
    {
        Additive,
        Blend,
        Override,
        Animation,
    };


    struct Node
    {
        std::string Name;
        Node* Parent = nullptr;
        Node* Child[2] = { nullptr, nullptr };
        NodeType Type;
        std::vector<glm::mat4> Pose;
        float Weight = 0.f;

        Node* Next() {
            Node* next = this;

            if (next->Child[1] == nullptr) {
                // Node has no right child
                next = this;
                while (next->Parent != nullptr && next == next->Parent->Child[1]) {
                    next = next->Parent;
                }
                next = next->Parent;
            } else {
                // Find the leftmost node in the right subtree
                next = next->Child[1];
                while (next->Child[0] != nullptr) {
                    next = next->Child[0];
                }
            }

            return next;

        }
    };

  



    BlendTree(EntityWrapper ModelEntity, Skeleton* skeleton);
    ~BlendTree();

    std::vector<glm::mat4> GetBoneTransforms(Skeleton* skeleton);

    void PrintTree();

private:
    Node* m_Root = nullptr;

    void FillTree(Node* parentNode, EntityWrapper parentEntity, Skeleton* skeleton);
    BlendTree::Node* FillTreeByName(Node* parentNode, std::string name, EntityWrapper parentEntity, Skeleton* skeleton);

    void Blend(Skeleton* skeleton, std::vector<glm::mat4>& pose);
};

#endif
