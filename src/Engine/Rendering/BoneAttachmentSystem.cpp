#include "Rendering/BoneAttachmentSystem.h"

void BoneAttachmentSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& BoneAttachmentComponent, double dt)
{


    if(!entity.HasComponent("Transform")) {
        return;
    }

    auto parent = entity.FirstParentWithComponent("Model");

    if(!parent.Valid()) {
        return;
    }


    Model* model;
    try {
        model = ResourceManager::Load<::Model, true>(parent["Model"]["Resource"]);
    } catch (const std::exception&) {
        return;
    }

    if (!model->IsSkinned()) {
        return;
    }

    Skeleton* skeleton = model->m_RawModel->m_Skeleton;

    if(skeleton == nullptr) {
        return;
    }

    int id = skeleton->GetBoneID(entity["BoneAttachment"]["BoneName"]);

    if (id == -1) {
        return;
    }

    if (skeleton->BlendTrees.find(parent) != skeleton->BlendTrees.end()) {


        glm::mat4 boneTransform = skeleton->BlendTrees.at(parent)->GetBoneTransform(id);

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;
        glm::decompose(boneTransform, scale, rotation, translation, skew, perspective);

        rotation = glm::quat((glm::vec3)entity["BoneAttachment"]["OrientationOffset"]) * rotation;

        rotation.w = -rotation.w;
        glm::vec3 angles = glm::eulerAngles(rotation);
        
        if ((bool)entity["BoneAttachment"]["InheritPosition"]) {
            (glm::vec3&)entity["Transform"]["Position"] = translation + (glm::vec3)entity["BoneAttachment"]["PositionOffset"];
        }
        if ((bool)entity["BoneAttachment"]["InheritOrientation"]) {
            (glm::vec3&)entity["Transform"]["Orientation"] = angles;
        }
        if ((bool)entity["BoneAttachment"]["InheritScale"]) {
            (glm::vec3&)entity["Transform"]["Scale"] = scale  * (glm::vec3)entity["BoneAttachment"]["ScaleOffset"];
        }
    }

}
