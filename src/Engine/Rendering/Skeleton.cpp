#include "Rendering/Skeleton.h"

std::map<int, glm::mat4> Skeleton::GetFrameBones(const Animation* animation, double time, bool additive, bool noRootMotion /*= false*/)
{
    if (animation == nullptr) {
        std::map<int, glm::mat4> finalMatrices;
        for (auto& b : Bones) {
            finalMatrices[b.second->ID] = glm::mat4(1);
        }
        return finalMatrices;
    }


    std::map<int, glm::mat4> frameBones;

    if(!additive) {
        AccumulateBoneTransforms(true, animation, time, frameBones, RootBone, glm::mat4(1));
    } else {
        AdditiveBoneTransforms(animation, time, frameBones, RootBone);
    }

    return frameBones;
}

void Skeleton::AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, double time, std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix)
{
    glm::mat4 boneMatrix;
    

    if (animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) {
        std::vector<Animation::Keyframe> boneKeyFrames = animation->JointAnimations.at(bone->ID);

        Animation::Keyframe currentFrame;
        Animation::Keyframe nextFrame;

        if (boneKeyFrames.size() > 1) { // 2+ keyframes for the current bone
            for (int index = boneKeyFrames.size()-1; index >= 0; index--) { // find the bone keyframes that surrounds the current frame
                if (time >= boneKeyFrames.at(index).Time) {
                    currentFrame = boneKeyFrames.at(index);
                    nextFrame = boneKeyFrames.at((index + 1) % boneKeyFrames.size());
                    break;
                }
            }

            float progress;

            if (nextFrame.Index == 0) {
                nextFrame = currentFrame;
                progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
            } else {
                progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);

            }

            progress = glm::clamp(progress, 0.0f, 1.0f);
            Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties;
            Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties;

            glm::vec3 position = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
            glm::quat rotation = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
            glm::vec3 scale = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

            // Flag for no root motion
            if (bone == RootBone && noRootMotion) {
                position.x = 0;
                position.z = 0;
            }

            boneMatrix = (glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale));
            boneMatrices[bone->ID] = boneMatrix;// *bone->OffsetMatrix;

        } else { // 1 keyframes for the current bone
            currentFrame = boneKeyFrames.at(0);
            boneMatrix =  (glm::translate(currentFrame.BoneProperties.Position) * glm::toMat4(currentFrame.BoneProperties.Rotation) * glm::scale(currentFrame.BoneProperties.Scale));
            boneMatrices[bone->ID] = boneMatrix;// *bone->OffsetMatrix;
        }
    } else { // 0 keyframes for the current bone
        if (bone->Parent) {
            //boneMatrix = parentMatrix * (glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix);
            //boneMatrices[bone->ID] = boneMatrix *bone->OffsetMatrix;
        } else {
            //boneMatrix = glm::inverse(bone->OffsetMatrix);
            //boneMatrices[bone->ID] = parentMatrix;
        }
    }

    for (auto &child : bone->Children) {
        AccumulateBoneTransforms(noRootMotion, animation, time, boneMatrices, child, boneMatrix);
    }
}


void Skeleton::AdditiveBoneTransforms(const Animation* animation, double time, std::map<int, glm::mat4>& boneMatrices, const Bone* bone)
{
    if (animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) {
        glm::mat4 refPose = GetAdditiveBonePose(bone, animation, 0.0);
        glm::mat4 srcPose = GetAdditiveBonePose(bone, animation, time + 1.0/60.0);
        glm::mat4 boneMatrix = srcPose * glm::inverse(refPose);
        boneMatrices[bone->ID] = boneMatrix;
    }

    for (auto &child : bone->Children) {
        AdditiveBoneTransforms(animation, time, boneMatrices, child);
    }
}

glm::mat4 Skeleton::GetAdditiveBonePose(const Bone* bone, const Animation* animation, double time)
{
    glm::vec3 position = glm::vec3(0);
    glm::quat rotation = glm::quat();
    glm::vec3 scale = glm::vec3(1);

    if (animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) {
        std::vector<Animation::Keyframe> boneKeyFrames = animation->JointAnimations.at(bone->ID);

        Animation::Keyframe currentFrame;
        Animation::Keyframe nextFrame;

        if (boneKeyFrames.size() > 1) { // 2+ keyframes for the current bone
            for (int index = boneKeyFrames.size()-1; index >= 0; index--) { // find the bone keyframes that surrounds the current frame
                if (time >= boneKeyFrames.at(index).Time) {
                    currentFrame = boneKeyFrames.at(index);
                    nextFrame = boneKeyFrames.at((index + 1) % boneKeyFrames.size());
                    break;
                }
            }

            float progress;

            if (nextFrame.Index == 0) {
                nextFrame = currentFrame;
                progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
            } else {
                progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);
            }

            progress = glm::clamp(progress, 0.0f, 1.0f);
            
            Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties;
            Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties;

            position = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
            rotation = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
            scale = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;


        } else { // 1 keyframes for the current bone
            currentFrame = boneKeyFrames.at(0);
            position = currentFrame.BoneProperties.Position;
            rotation = currentFrame.BoneProperties.Rotation;
            scale = currentFrame.BoneProperties.Scale;
        }
    }

    return (glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale));;
}


glm::mat4 Skeleton::GetBonePose(const Bone* bone, const Animation* animation, double time, bool noRootMotion)
{
    glm::mat4 boneMatrix;

    if (animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) {
        std::vector<Animation::Keyframe> boneKeyFrames = animation->JointAnimations.at(bone->ID);

        Animation::Keyframe currentFrame;
        Animation::Keyframe nextFrame;

        if (boneKeyFrames.size() > 1) { // 2+ keyframes for the current bone
            for (int index = boneKeyFrames.size()-1; index >= 0; index--) { // find the bone keyframes that surrounds the current frame
                if (time >= boneKeyFrames.at(index).Time) {
                    currentFrame = boneKeyFrames.at(index);
                    nextFrame = boneKeyFrames.at((index + 1) % boneKeyFrames.size());
                    break;
                }
            }

            float progress;

            if (nextFrame.Index == 0) {
                nextFrame = currentFrame;
                progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
            } else {
                progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);

            }


            if (progress > 1.0f || progress < 0.0f) {
                progress = glm::clamp(progress, 0.0f, 1.0f);
            }
            Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties;
            Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties;

            glm::vec3 position = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
            glm::quat rotation = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
            glm::vec3 scale = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

            // Flag for no root motion
            if (bone == RootBone && noRootMotion) {
                position.x = 0;
                position.z = 0;
            }

            boneMatrix = (glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale));

        } else { // 1 keyframes for the current bone
            currentFrame = boneKeyFrames.at(0);
            boneMatrix = (glm::translate(currentFrame.BoneProperties.Position) * glm::toMat4(currentFrame.BoneProperties.Rotation) * glm::scale(currentFrame.BoneProperties.Scale));
        }
    } //else { // 0 keyframes for the current bone

   // }

    return boneMatrix;
}

glm::mat4 Skeleton::GetBoneTransform(const Bone* bone, const Animation* animation, float time, glm::mat4 childMatrix)
{
    glm::mat4 boneMatrix;

    Animation::Keyframe currentFrame;
    Animation::Keyframe nextFrame;

    if (animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) {
        std::vector<Animation::Keyframe> boneKeyFrames = animation->JointAnimations.at(bone->ID);

        if (boneKeyFrames.size() > 1) { // 2+ keyframes for the current bone
            for (int index = boneKeyFrames.size()-1; index >= 0; index--) { // find the bone keyframes that surrounds the current frame
                if (time >= boneKeyFrames.at(index).Time) {
                    currentFrame = boneKeyFrames.at(index);
                    nextFrame = boneKeyFrames.at((index + 1) % boneKeyFrames.size());
                    break;
                }
            }

            float progress;

            if (nextFrame.Index == 0) {
                nextFrame = currentFrame;
                progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
            } else {
                progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);
            }

            if (progress > 1.0f || progress < 0.0f) {
                LOG_INFO("Progress %f", progress);
                progress = glm::clamp(progress, 0.0f, 1.0f);
            }
            Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties;
            Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties;

            glm::vec3 positionInterp = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
            glm::quat rotationInterp = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
            glm::vec3 scaleInterp = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

            boneMatrix = (glm::translate(positionInterp) * glm::toMat4(rotationInterp) * glm::scale(scaleInterp)) * childMatrix;

        } else { // 1 keyframes for the current bone
            currentFrame = boneKeyFrames.at(0);
            boneMatrix = (glm::translate(currentFrame.BoneProperties.Position) * glm::toMat4(currentFrame.BoneProperties.Rotation) * glm::scale(currentFrame.BoneProperties.Scale)) * childMatrix;

        }
    } else { // 0 keyframes for the current bone
        if (bone->Parent) {
            boneMatrix = bone->Parent->OffsetMatrix * glm::inverse(bone->OffsetMatrix) * childMatrix;
        } else {
            boneMatrix = glm::inverse(bone->OffsetMatrix) * childMatrix;
        }
    }

    if (bone->Parent) {
        return  GetBoneTransform(bone->Parent, animation, time, boneMatrix);
    } else {
        return boneMatrix;
    }
}

std::map<int, glm::mat4> Skeleton::BlendPoses(const std::map<int, glm::mat4>& pose1, const std::map<int, glm::mat4>& pose2, float weight)
{
    std::map<int, glm::mat4> finalPose;

    for (auto& b : Bones) {
        int boneID = b.second->ID;
        glm::mat4 blendedPose = glm::mat4(0);

        if(pose1.find(boneID) != pose1.end() && pose2.find(boneID) != pose2.end()) {
            blendedPose += pose1.at(boneID) * weight;
            blendedPose += pose2.at(boneID) * (1.f - weight);
            finalPose[boneID] = blendedPose;
        } else if(pose1.find(boneID) != pose1.end()) {
            finalPose[boneID] = pose1.at(boneID);
        } else if (pose2.find(boneID) != pose2.end()) {
            finalPose[boneID] = pose2.at(boneID);
        }
    }

    return finalPose;
}

std::map<int, glm::mat4> Skeleton::OverridePose(const std::map<int, glm::mat4>& overridePose, const std::map<int, glm::mat4>& targetPose)
{
    std::map<int, glm::mat4> finalPose;

    for (auto& b : Bones) {
        int boneID = b.second->ID;
        if (overridePose.find(boneID) != overridePose.end()) {
            finalPose[boneID] = overridePose.at(boneID);
        } else if (targetPose.find(boneID) != targetPose.end()) {
            finalPose[boneID] = targetPose.at(boneID);
        }
    }
    return finalPose;
}

std::map<int, glm::mat4> Skeleton::BlendPoseAdditive(const std::map<int, glm::mat4>& additivePose, const std::map<int, glm::mat4>& targetPose)
{
    std::map<int, glm::mat4> finalPose;

    for (auto& b : Bones) {
        int boneID = b.second->ID;
        glm::mat4 blendedPose = glm::mat4(1);

        if (additivePose.find(boneID) != additivePose.end() && targetPose.find(boneID) != targetPose.end()) {
            blendedPose = additivePose.at(boneID) * targetPose.at(boneID);
            finalPose[boneID] = blendedPose;

        } else if (additivePose.find(boneID) != additivePose.end()) {
            finalPose[boneID] = additivePose.at(boneID);
        } else if (targetPose.find(boneID) != targetPose.end()) {
            finalPose[boneID] = targetPose.at(boneID);
        }
    }

    return finalPose;
}

void Skeleton::GetFinalPose(std::map<int, glm::mat4>& boneMatrices, std::vector<glm::mat4>& finalPose, std::map<int, glm::mat4>& boneTransforms)
{
    AccumulateFinalPose(boneMatrices, boneTransforms, RootBone, glm::mat4(1));

    for(auto& b : boneMatrices) {
        finalPose.push_back(b.second);
    }

}

void Skeleton::AccumulateFinalPose(std::map<int, glm::mat4>& boneMatrices, std::map<int, glm::mat4>& boneTransforms, const Bone* bone, glm::mat4 parentMatrix)
{

    glm::mat4 boneMatrix;


    if (boneMatrices.find(bone->ID) != boneMatrices.end()) {

        boneMatrix = parentMatrix * boneMatrices.at(bone->ID);
        boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;

    } else {
        if (bone->Parent) {
            boneMatrix = parentMatrix * (glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix);
            boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
        } else {
            boneMatrix = glm::inverse(bone->OffsetMatrix);
            boneMatrices[bone->ID] = parentMatrix;
        }
    }

    boneTransforms[bone->ID] = boneMatrix;

    for (auto &child : bone->Children) {
        AccumulateFinalPose(boneMatrices, boneTransforms, child, boneMatrix);
    }
}

int Skeleton::GetBoneID(std::string name)
{
	if (m_BonesByName.find(name) == m_BonesByName.end()) {
		return -1;
	} else {
		return m_BonesByName.at(name)->ID;
	}
}

int Skeleton::CreateBone(int ID, int parentID, std::string name, glm::mat4 offsetMatrix)
{
    if (m_BonesByName.find(name) != m_BonesByName.end()) {
        return m_BonesByName.at(name)->ID;
    } else {
        Bone* bone;

        if (parentID == -1) {
            bone = new Bone(ID, nullptr, name, offsetMatrix);
            RootBone = bone;
        } else {
            Bone* parent = Bones[parentID];
            bone = new Bone(ID, parent, name, offsetMatrix);
            parent->Children.push_back(bone);
        }

        Bones[ID] = bone;
        m_BonesByName[name] = bone;
        return ID;
    }
}

Skeleton::~Skeleton()
{
    for (auto &kv : Bones) {
        delete kv.second;
    }
}

const Skeleton::Animation* Skeleton::GetAnimation(std::string name)
{
    auto it = Animations.find(name);
    if (it != Animations.end()) {
        return const_cast<const Animation*>(&it->second);
    } else {
        return nullptr;
    }
}