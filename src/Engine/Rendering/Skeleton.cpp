#include "Rendering/Skeleton.h"

std::map<int, Skeleton::PoseData> Skeleton::GetFrameBones(const Animation* animation, double time, bool additive, bool noRootMotion /*= false*/)
{
    if (animation == nullptr) {
        std::map<int, PoseData> finalMatrices;
        for (auto& b : Bones) {
            PoseData poseData;
            poseData.Translation = glm::vec3(0);
            poseData.Orientation = glm::quat();
            poseData.Scale = glm::vec3(1);


            finalMatrices[b.second->ID] = poseData;
        }
        return finalMatrices;
    }


    std::map<int, PoseData> frameBones;

    if(!additive) {
        AccumulateBoneTransforms(true, animation, time, frameBones, RootBone);
    } else {
        AdditiveBoneTransforms(animation, time, frameBones, RootBone);
    }

    return frameBones;
}

void Skeleton::AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, double time, std::map<int, PoseData>& boneMatrices, const Bone* bone)
{
    PoseData poseData;
    
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
            glm::quat rotation = glm::normalize(glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress));
            glm::vec3 scale = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

            // Flag for no root motion
            if (bone == RootBone && noRootMotion) {
                position.x = 0;
                position.z = 0;
            }

            poseData.Translation = position;
            poseData.Orientation = rotation;
            poseData.Scale = scale;
            boneMatrices[bone->ID] = poseData;

        } else { // 1 keyframes for the current bone
            currentFrame = boneKeyFrames.at(0);
            poseData.Translation = currentFrame.BoneProperties.Position;
            poseData.Orientation = currentFrame.BoneProperties.Rotation;
            poseData.Scale = currentFrame.BoneProperties.Scale;
            boneMatrices[bone->ID] = poseData;
        }
    }

    for (auto &child : bone->Children) {
        AccumulateBoneTransforms(noRootMotion, animation, time, boneMatrices, child);
    }
}


void Skeleton::AdditiveBoneTransforms(const Animation* animation, double time, std::map<int, PoseData>& boneMatrices, const Bone* bone)
{
    if (animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) {
        PoseData refPose = GetAdditiveBonePose(bone, animation, 0.0);
        PoseData srcPose = GetAdditiveBonePose(bone, animation, time + 1.0/60.0);
        
        PoseData finalPose;
        finalPose.Translation = srcPose.Translation - refPose.Translation;
        finalPose.Orientation = srcPose.Orientation * glm::inverse(refPose.Orientation);
        finalPose.Scale = srcPose.Scale - refPose.Scale;

        boneMatrices[bone->ID] = finalPose;
    }

    for (auto &child : bone->Children) {
        AdditiveBoneTransforms(animation, time, boneMatrices, child);
    }
}

Skeleton::PoseData Skeleton::GetAdditiveBonePose(const Bone* bone, const Animation* animation, double time)
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

    PoseData finalPose;
    finalPose.Translation = position;
    finalPose.Orientation = rotation;
    finalPose.Scale = scale;

    return finalPose;
}

std::map<int, Skeleton::PoseData> Skeleton::BlendPoses(const std::map<int, PoseData>& pose1, const std::map<int, PoseData>& pose2, double weight)
{
    std::map<int, PoseData> finalPose;

    float weight1 = (float)(1.0 - weight);
    float weight2 = (float)(weight);

    for (auto& b : Bones) {
        int boneID = b.second->ID;
        PoseData blendedPose;
        blendedPose.Translation = glm::vec3(0);
        blendedPose.Orientation = glm::quat();
        blendedPose.Scale = glm::vec3(1);

        if(pose1.find(boneID) != pose1.end() && pose2.find(boneID) != pose2.end()) {
            blendedPose.Translation = pose1.at(boneID).Translation * weight1 + pose2.at(boneID).Translation * weight2;
            blendedPose.Orientation = glm::slerp(pose1.at(boneID).Orientation, pose2.at(boneID).Orientation, weight2);
            blendedPose.Scale = pose1.at(boneID).Scale * weight1 + pose2.at(boneID).Scale * weight2;
            finalPose[boneID] = blendedPose;
        } else if(pose1.find(boneID) != pose1.end()) {
            finalPose[boneID] = pose1.at(boneID);
        } else if (pose2.find(boneID) != pose2.end()) {
            finalPose[boneID] = pose2.at(boneID);
        }
    }

    return finalPose;
}

std::map<int, Skeleton::PoseData> Skeleton::OverridePose(const std::map<int, PoseData>& overridePose, const std::map<int, PoseData>& targetPose)
{
    std::map<int, PoseData> finalPose;

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

std::map<int, Skeleton::PoseData> Skeleton::BlendPoseAdditive(const std::map<int, PoseData>& additivePose, const std::map<int, PoseData>& targetPose)
{
    std::map<int, PoseData> finalPose;

    for (auto& b : Bones) {
        int boneID = b.second->ID;
        PoseData blendedPose;
        blendedPose.Translation = glm::vec3(0);
        blendedPose.Orientation = glm::quat();
        blendedPose.Scale = glm::vec3(1);

        if (additivePose.find(boneID) != additivePose.end() && targetPose.find(boneID) != targetPose.end()) {
            blendedPose.Translation = additivePose.at(boneID).Translation + targetPose.at(boneID).Translation;
            blendedPose.Orientation = additivePose.at(boneID).Orientation * targetPose.at(boneID).Orientation;
            blendedPose.Scale = additivePose.at(boneID).Scale + targetPose.at(boneID).Scale;
            finalPose[boneID] = blendedPose;
        } else if (additivePose.find(boneID) != additivePose.end()) {
            finalPose[boneID] = additivePose.at(boneID);
        } else if (targetPose.find(boneID) != targetPose.end()) {
            finalPose[boneID] = targetPose.at(boneID);
        }
    }

    return finalPose;
}

void Skeleton::GetFinalPose(std::map<int, Skeleton::PoseData>& poseDatas, std::vector<glm::mat4>& finalPose, std::map<int, glm::mat4>& boneTransforms)
{

    std::map<int, glm::mat4> boneMatrices;

    AccumulateFinalPose(boneMatrices, poseDatas, boneTransforms, RootBone, glm::mat4(1));

    for(auto& b : boneMatrices) {
        finalPose.push_back(b.second);
    }

}


std::vector<glm::mat4> Skeleton::GetTPose()
{
    std::vector<glm::mat4> finalMatrices;

    for (auto b : Bones) {
        finalMatrices.push_back(glm::mat4(1));
    }

    return finalMatrices;
}

void Skeleton::AccumulateFinalPose(std::map<int, glm::mat4>& boneMatrices, std::map<int, Skeleton::PoseData>& poseDatas, std::map<int, glm::mat4>& boneTransforms, const Bone* bone, glm::mat4 parentMatrix)
{
    glm::mat4 boneMatrix;

    if (poseDatas.find(bone->ID) != poseDatas.end()) {
        boneMatrix = parentMatrix * (glm::translate(poseDatas.at(bone->ID).Translation) * glm::mat4(poseDatas.at(bone->ID).Orientation) * glm::scale(poseDatas.at(bone->ID).Scale));
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
        AccumulateFinalPose(boneMatrices, poseDatas, boneTransforms, child, boneMatrix);
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

    BlendTrees.clear();
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