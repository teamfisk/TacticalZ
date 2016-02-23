#include "Rendering/BlendTree.h"

BlendTree::BlendTree(EntityWrapper ModelEntity, Skeleton* skeleton)
{
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
        m_Root->Pose = skeleton->GetFrameBones(animation, (double)ModelEntity["Animation"]["Time"], (bool)ModelEntity["Animation"]["Additive"]);
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Animation;

    } else if (ModelEntity.HasComponent("Blend")) {
        m_Root = new Node();
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Blend;
        m_Root->Weight = (double)ModelEntity["Blend"]["Weight"];
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["Blend"]["Pose1"], ModelEntity, skeleton);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["Blend"]["Pose2"], ModelEntity, skeleton);

    } else if (ModelEntity.HasComponent("BlendOverride")) {
        m_Root = new Node();
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Override;
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendOverride"]["Master"], ModelEntity, skeleton);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendOverride"]["Slave"], ModelEntity, skeleton);

    } else if (ModelEntity.HasComponent("BlendAdditive")) {
        m_Root = new Node();
        m_Root->Name = ModelEntity.Name();
        m_Root->Parent = nullptr;
        m_Root->Type = NodeType::Additive;
        m_Root->Child[0] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendAdditive"]["Adder"], ModelEntity, skeleton);
        m_Root->Child[1] = FillTreeByName(m_Root, (std::string)ModelEntity["BlendAdditive"]["Receiver"], ModelEntity, skeleton);
    }



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
        currentNode = currentNode->Next();
        m_NodesToRemove.push_back(currentNode);
    }

    for (auto it = m_NodesToRemove.begin(); it != m_NodesToRemove.end(); it++) {
        if ((*it) != nullptr) {
            delete (*it);
            (*it) = nullptr;
        }
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



void BlendTree::FillTree(Node* parentNode, EntityWrapper parentEntity, Skeleton* skeleton)
{
    auto itPair = parentEntity.World->GetChildren(parentEntity.ID);
    if (itPair.first == itPair.second) {
        return; // no children
    }

    unsigned int childIndex = 0;
    for (auto it = itPair.first; it != itPair.second; ++it) {
        
        EntityWrapper childEntity = EntityWrapper(parentEntity.World, it->second);
        
        if(!childEntity.Valid()) {
            continue;
        }

        if (childEntity.HasComponent("Animation")) {
            const Skeleton::Animation* animation = skeleton->GetAnimation(childEntity["Animation"]["AnimationName"]);
            if(animation == nullptr) {
                continue;
            }

            Node* node = new Node();
            node->Name = childEntity.Name();
            node->Pose = skeleton->GetFrameBones(animation, (double)childEntity["Animation"]["Time"], (bool)childEntity["Animation"]["Additive"]);
            node->Parent = parentNode;
            node->Type = NodeType::Animation;
            parentNode->Child[childIndex] = node;
            childIndex++;
            FillTree(node, childEntity, skeleton);

        } else if (childEntity.HasComponent("Blend")) {
            Node* node = new Node();
            node->Name = childEntity.Name();
            node->Parent = parentNode;
            node->Type = NodeType::Blend;
            (double&)childEntity["Blend"]["Weight"] = glm::clamp((float)(double)childEntity["Blend"]["Weight"], 0.f, 1.f);
            node->Weight = (double)childEntity["Blend"]["Weight"];
            parentNode->Child[childIndex] = node;
            childIndex++;
            FillTree(node, childEntity, skeleton);

        } else if (childEntity.HasComponent("BlendOverride")) {
            Node* node = new Node();
            node->Name = childEntity.Name();
            node->Parent = parentNode;
            node->Type = NodeType::Override;
            parentNode->Child[childIndex] = node;
            childIndex++;
            FillTree(node, childEntity, skeleton);

        } else if (childEntity.HasComponent("BlendAdditive")) {
            Node* node = new Node();
            node->Name = childEntity.Name();
            node->Parent = parentNode;
            node->Type = NodeType::Additive;
            parentNode->Child[childIndex] = node;
            childIndex++;
            FillTree(node, childEntity, skeleton);
        }
    }
}


BlendTree::Node* BlendTree::FillTreeByName(Node* parentNode, std::string name, EntityWrapper parentEntity, Skeleton* skeleton)
{
    EntityWrapper childEntity = parentEntity.FirstChildByName(name);

    if (!childEntity.Valid()) {
        return nullptr;
    }

    if (childEntity.HasComponent("Animation")) {
        const Skeleton::Animation* animation = skeleton->GetAnimation(childEntity["Animation"]["AnimationName"]);
        if (animation == nullptr) {
            return nullptr;
        }

        Node* node = new Node();
        node->Name = childEntity.Name();
        node->Pose = skeleton->GetFrameBones(animation, (double)childEntity["Animation"]["Time"], (bool)childEntity["Animation"]["Additive"]);
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
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose1"], childEntity, skeleton);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["Blend"]["Pose2"], childEntity, skeleton);
        return node;
    } else if (childEntity.HasComponent("BlendOverride")) {
        Node* node = new Node();
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Override;
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["BlendOverride"]["Master"], childEntity, skeleton);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["BlendOverride"]["Slave"], childEntity, skeleton);
        return node;
    } else if (childEntity.HasComponent("BlendAdditive")) {
        Node* node = new Node();
        node->Name = childEntity.Name();
        node->Parent = parentNode;
        node->Type = NodeType::Additive;
        node->Child[0] = FillTreeByName(node, (std::string)childEntity["BlendAdditive"]["Adder"], childEntity, skeleton);
        node->Child[1] = FillTreeByName(node, (std::string)childEntity["BlendAdditive"]["Receiver"], childEntity, skeleton);
        return node;
    }
    
    return nullptr;
}

void BlendTree::Blend(Skeleton* skeleton, std::map<int, glm::mat4>& pose)
{
    Node* currentNode;
    Node* start = m_Root;
    while (start->Child[0] != nullptr) {
        start = start->Child[0];
    }

    currentNode = start;
    LOG_INFO("\n\n");
    while (m_Root->Pose.size() == 0) {
        if(currentNode->Pose.size() == 0) {
            if (currentNode->Child[0] != nullptr && currentNode->Child[1] != nullptr) {
                if (currentNode->Child[0]->Pose.size() != 0 && currentNode->Child[1]->Pose.size() != 0) {

                    switch (currentNode->Type) {
                    case BlendTree::NodeType::Additive:
                        currentNode->Pose = skeleton->BlendPoseAdditive(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose);
                        break;
                    case BlendTree::NodeType::Blend:
                        currentNode->Pose = skeleton->BlendPoses(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose, currentNode->Weight);
                        break;
                    case BlendTree::NodeType::Override:
                        currentNode->Pose = skeleton->OverridePose(currentNode->Child[0]->Pose, currentNode->Child[1]->Pose);
                        break;
                    case BlendTree::NodeType::Animation:
                        // do nothing
                        break;
                    } 

                    LOG_INFO("Blending %s and %s", currentNode->Child[0]->Name.c_str(), currentNode->Child[1]->Name.c_str());
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

std::vector<glm::mat4> BlendTree::GetBoneTransforms(Skeleton* skeleton)
{
    std::vector<glm::mat4> finalPose;
    if (skeleton == nullptr || m_Root == nullptr) {
        
        for (int i = 0; i < skeleton->Bones.size(); i++) {
            finalPose.push_back(glm::mat4(1));
        }
        return finalPose;
    }

    std::map<int, glm::mat4> pose;
    Blend(skeleton, pose);

    finalPose = skeleton->GetFinalPose(pose);

    return finalPose;
}

