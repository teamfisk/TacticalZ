#include "Rendering/BlendTree.h"

BlendTree::BlendTree(EntityWrapper ModelEntity, Skeleton* skeleton)
{

    m_Skeleton = skeleton;


    auto itPair = ModelEntity.World->GetChildren(ModelEntity.ID);
    if (itPair.first == itPair.second) {
        return;
    }


    if (ModelEntity.HasComponent("Animation")) {
        const Skeleton::Animation* animation = skeleton->GetAnimation(ModelEntity["Animation"]["AnimationName"]);
        if (animation == nullptr) {
            return;
        }

        m_Root = new Node();
        m_Root->Name = ModelEntity.Name();
        m_Root->Pose = m_Skeleton->GetFrameBones(animation, (double)ModelEntity["Animation"]["Time"], (bool)ModelEntity["Animation"]["Additive"]);
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Animation;

    } else if (ModelEntity.HasComponent("Blend")) {
        m_Root = new Node();
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Blend;
        m_Root->Weight = (double)ModelEntity["Blend"]["Weight"];
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["Blend"]["Pose1"], ModelEntity);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["Blend"]["Pose2"], ModelEntity);

    } else if (ModelEntity.HasComponent("BlendOverride")) {
        m_Root = new Node();
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Override;
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendOverride"]["Master"], ModelEntity);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendOverride"]["Slave"], ModelEntity);

    } else if (ModelEntity.HasComponent("BlendAdditive")) {
        m_Root = new Node();
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Additive;
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendAdditive"]["Adder"], ModelEntity);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendAdditive"]["Receiver"], ModelEntity);
    }

    m_FinalPose = AccumulateFinalPose();


   // PrintTree();
}

BlendTree::~BlendTree()
{
    Node* currentNode = m_Root;

    while (currentNode->Child[0] != nullptr) {
        currentNode = currentNode->Child[0];
    }

    std::list<Node*> m_NodesToRemove;

    while (currentNode != nullptr) {
        m_NodesToRemove.push_back(currentNode);
        currentNode = currentNode->Next();
    }

    for (auto it = m_NodesToRemove.begin(); it != m_NodesToRemove.end(); it++) {
        delete (*it);
    }
}


glm::mat4 BlendTree::GetBoneTransform(int boneID)
{
    if(m_FinalBoneTransforms.find(boneID) != m_FinalBoneTransforms.end()) {
        return m_FinalBoneTransforms.at(boneID);
    } else {
        return glm::mat4(1);
    }
    
}

void BlendTree::PrintTree()
{
    Node* currentNode = m_Root;
    LOG_INFO("\n\n");

    while(currentNode->Child[0] != nullptr) {
        currentNode = currentNode->Child[0];
    }

    while (currentNode != nullptr)
    {
        LOG_INFO("%s", currentNode->Name.c_str());
        currentNode = currentNode->Next();
    }

    
}

BlendTree::Node* BlendTree::FillTreeByName(Node* parentNode, std::string name, EntityWrapper parentEntity)
{
    EntityWrapper childEntity = parentEntity.FirstChildByName(name); // Make first level child by name

    if (!childEntity.Valid()) {
        return nullptr;
    }

    if (childEntity.HasComponent("Animation")) {
        const Skeleton::Animation* animation = m_Skeleton->GetAnimation(childEntity["Animation"]["AnimationName"]);
        if (animation == nullptr) {
            return nullptr;
        }

        Node* node = new Node();
        node->Name = childEntity.Name();
        node->Pose = m_Skeleton->GetFrameBones(animation, (double)childEntity["Animation"]["Time"], (bool)childEntity["Animation"]["Additive"]);
        node->Parent = parentNode;
        node->Type = NodeType::Animation;
        return node;

    } else if (childEntity.HasComponent("Blend")) {
        Node* node = new Node();
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Blend;
        (double&)childEntity["Blend"]["Weight"] = glm::clamp((float)(double)childEntity["Blend"]["Weight"], 0.f, 1.f);
        node->Weight = (double)childEntity["Blend"]["Weight"];
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose1"], childEntity);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose2"], childEntity);
        return node;
    } else if (childEntity.HasComponent("BlendOverride")) {
        Node* node = new Node();
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Override;
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["BlendOverride"]["Master"], childEntity);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["BlendOverride"]["Slave"], childEntity);
        return node;
    } else if (childEntity.HasComponent("BlendAdditive")) {
        Node* node = new Node();
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Additive;
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["BlendAdditive"]["Adder"], childEntity);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["BlendAdditive"]["Receiver"], childEntity);
        return node;
    }
    

    return nullptr;
}

void BlendTree::Blend(std::map<int, glm::mat4>& pose)
{
    Node* currentNode;
    Node* start = m_Root;
    while (start->Child[0] != nullptr) {
        start = start->Child[0];
    }

    currentNode = start;

    while (m_Root->Pose.size() == 0) {
        if(currentNode->Pose.size() == 0) {
            if (currentNode->Child[0] != nullptr && currentNode->Child[1] != nullptr) {
                if (currentNode->Child[0]->Pose.size() != 0 && currentNode->Child[1]->Pose.size() != 0) {

                    switch (currentNode->Type) {
                    case BlendTree::NodeType::Additive:
                        currentNode->Pose = m_Skeleton->BlendPoseAdditive(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose);
                        break;
                    case BlendTree::NodeType::Blend:
                        currentNode->Pose = m_Skeleton->BlendPoses(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose, currentNode->Weight);
                        break;
                    case BlendTree::NodeType::Override:
                        currentNode->Pose = m_Skeleton->OverridePose(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose);
                        break;
                    case BlendTree::NodeType::Animation:
                        // do nothing
                        break;
                    } 
                } 
            } else if (currentNode->Child[0] != nullptr) {
                if (currentNode->Child[0]->Pose.size() != 0) {
                    currentNode->Pose = currentNode->Child[0]->Pose;
                }
            } else if (currentNode->Child[1] != nullptr) {
                if (currentNode->Child[1]->Pose.size() != 0) {
                    currentNode->Pose = currentNode->Child[1]->Pose;
                }
            }
        } 

        currentNode = currentNode->Next();

        if (currentNode == nullptr) {
            currentNode = start;
        }
        
    }

    pose = m_Root->Pose;
}

std::vector<glm::mat4> BlendTree::AccumulateFinalPose()
{
    std::vector<glm::mat4> finalPose;
    if (m_Skeleton == nullptr || m_Root == nullptr) {
        
        for (int i = 0; i < m_Skeleton->Bones.size(); i++) {
            finalPose.push_back(glm::mat4(1));
        }
        return finalPose;
    }

    std::map<int, glm::mat4> pose;
    Blend(pose);

    m_Skeleton->GetFinalPose(pose, finalPose, m_FinalBoneTransforms);

    return finalPose;
}

