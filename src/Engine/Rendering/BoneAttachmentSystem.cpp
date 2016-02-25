#include "Rendering/BoneAttachmentSystem.h"

void BoneAttachmentSystem::UpdateComponent(EntityWrapper& entity, ComponentWrapper& BoneAttachmentComponent, double dt)
{


    if(!entity.HasComponent("Transform")) {
        return;
    }

    auto parent = entity.FirstParentWithComponent("Animation");
    if (!parent.HasComponent("Model")) {
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

   /* std::vector<::Skeleton::AnimationData> Animations;
    ::Skeleton::AnimationOffset AnimationOffset;
    glm::mat4 boneTransform;

    if (parent.HasComponent("Animation")) {
        ::Skeleton::AnimationData animationData;
        animationData.animation = model->m_RawModel->m_Skeleton->GetAnimation(parent["Animation"]["AnimationName"]);
        if (animationData.animation != nullptr) {
            animationData.time = (double)parent["Animation"]["Time"];
            Animations.push_back(animationData);
        }
    }

    if (parent.HasComponent("AnimationOffset")) {
        AnimationOffset.animation = model->m_RawModel->m_Skeleton->GetAnimation(parent["AnimationOffset"]["AnimationName"]);
        AnimationOffset.time = (double)parent["AnimationOffset"]["Time"];

        if(AnimationOffset.animation != nullptr) {
            boneTransform = skeleton->GetBoneTransform(false, skeleton->Bones.at(id), Animations, AnimationOffset, glm::mat4(1));
        } else {
            boneTransform = skeleton->GetBoneTransform(false, skeleton->Bones.at(id), Animations, glm::mat4(1));
        }
        
    } else {
        boneTransform = skeleton->GetBoneTransform(false, skeleton->Bones.at(id), Animations, glm::mat4(1));
    }
    
    

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(boneTransform, scale, rotation, translation, skew, perspective);

    glm::vec3 angles = glm::vec3(-glm::pitch(rotation), -glm::yaw(rotation), -glm::roll(rotation));
/ *

    angles.y = asin(-boneTransform[0][2]);
    if (cos(angles.y) != 0) {
        angles.x = atan2(boneTransform[1][2], boneTransform[2][2]);
        angles.z = atan2(boneTransform[0][1], boneTransform[0][0]);
    } else {
        angles.x = atan2(-boneTransform[2][0], boneTransform[1][1]);
        angles.z = 0;
    }* /

    if ((bool)entity["BoneAttachment"]["InheritPosition"]) {
        (glm::vec3&)entity["Transform"]["Position"] = translation + (glm::vec3)entity["BoneAttachment"]["PositionOffset"];
    }
    if ((bool)entity["BoneAttachment"]["InheritOrientation"]) {
        (glm::vec3&)entity["Transform"]["Orientation"] = angles  + (glm::vec3)entity["BoneAttachment"]["OrientationOffset"];
    }
    if ((bool)entity["BoneAttachment"]["InheritScale"]) {
        (glm::vec3&)entity["Transform"]["Scale"] = scale  * (glm::vec3)entity["BoneAttachment"]["ScaleOffset"];
    }*/
}
