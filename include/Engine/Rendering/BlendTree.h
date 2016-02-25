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
        std::map<int, glm::mat4> Pose;
        //std::vector<glm::mat4> Pose;
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


    std::vector<glm::mat4> GetFinalPose() { return m_FinalPose; }



    void PrintTree();

private:
    Skeleton* m_Skeleton = nullptr;
    Node* m_Root = nullptr;

    std::vector<glm::mat4> m_FinalPose;
    std::vector<glm::mat4> AccumulateFinalPose();
    BlendTree::Node* FillTreeByName(Node* parentNode, std::string name, EntityWrapper parentEntity);

    void Blend(std::map<int, glm::mat4>& pose);
};

#endif
