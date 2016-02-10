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


    Skeleton* skeleton = model->m_RawModel->m_Skeleton;

    if(skeleton == nullptr) {
        return;
    }

    const Skeleton::Animation* animation = skeleton->GetAnimation(parent["Animation"]["AnimationName1"]);

    if (!animation) {
        return;
    }

    int id = skeleton->GetBoneID(entity["BoneAttachment"]["BoneName"]);

    if(id == -1) {
        return;
    }


    glm::mat4 boneTransform = skeleton->GetBoneTransform(skeleton->Bones[id], animation, (double)parent["Animation"]["Time1"], glm::mat4(1));

    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(boneTransform, scale, rotation, translation, skew, perspective);

    glm::vec3 angles;
    angles.y = asin(-boneTransform[0][2]);
    if (cos(angles.y) != 0) {
        angles.x = atan2(boneTransform[1][2], boneTransform[2][2]);
        angles.z = atan2(boneTransform[0][1], boneTransform[0][0]);
    } else {
        angles.x = atan2(-boneTransform[2][0], boneTransform[1][1]);
        angles.z = 0;
    }

    if ((bool)entity["BoneAttachment"]["InheritPosition"]) {
        (glm::vec3&)entity["Transform"]["Position"] = translation + (glm::vec3)entity["BoneAttachment"]["PositionOffset"];
    }
    if ((bool)entity["BoneAttachment"]["InheritOrientation"]) {
        (glm::vec3&)entity["Transform"]["Orientation"] = angles  + (glm::vec3)entity["BoneAttachment"]["OrientationOffset"];
    }
    if ((bool)entity["BoneAttachment"]["InheritScale"]) {
        (glm::vec3&)entity["Transform"]["Scale"] = scale  * (glm::vec3)entity["BoneAttachment"]["ScaleOffset"];
    }
}
