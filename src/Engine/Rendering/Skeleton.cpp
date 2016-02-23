#include "Rendering/Skeleton.h"

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

std::vector<glm::mat4> Skeleton::GetFrameBones()
{
    std::vector<glm::mat4> finalMatrices;
    for (auto& b : Bones) {
        finalMatrices.push_back(glm::mat4(1));
    }
    return finalMatrices;
}
std::map<int, glm::mat4> Skeleton::GetFrameBones(const Animation* animation, const double time, bool additive, bool noRootMotion /*= false*/)
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
        AccumulateBoneTransforms(true, animation, time, frameBones, additive, RootBone, glm::mat4(1));
    } else {
        AdditiveBoneTransforms(animation, time, frameBones, RootBone);
    }

    return frameBones;
}

void Skeleton::AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, double time, std::map<int, glm::mat4>& boneMatrices, bool additive, const Bone* bone, glm::mat4 parentMatrix)
{
    if (additive) {
        time += 1.0/60.0; // first frame is a reference frame
    }

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
        AccumulateBoneTransforms(noRootMotion, animation, time, boneMatrices, additive, child, boneMatrix);
    }
}


void Skeleton::AdditiveBoneTransforms(const Animation* animation, double time, std::map<int, glm::mat4>& boneMatrices, const Bone* bone)
{
    if (animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) {
        glm::mat4 refPose = GetAdditiveBonePose(bone, animation, 0.0);
        glm::mat4 srcPose = GetAdditiveBonePose(bone, animation, time);
        glm::mat4 boneMatrix = srcPose * glm::inverse(refPose);
        boneMatrices[bone->ID] = boneMatrix;
    }

    for (auto &child : bone->Children) {
        AdditiveBoneTransforms(animation, time, boneMatrices, child);
    }
}



glm::mat4 Skeleton::AdditiveBlend(const Bone* bone, AnimationOffset animationOffset, glm::mat4 targetPose)
{
/*
    AnimationOffset refOffset = animationOffset;
    refOffset.time = 0.5f; // reference pose is at 0.5s for now
    glm::mat4 refPose = GetAdditiveBonePose(bone, refOffset);
    glm::mat4 srcPose = GetAdditiveBonePose(bone, animationOffset);
    glm::mat4 differencePose = srcPose * glm::inverse(refPose);
    glm::mat4 finalPose = differencePose * targetPose;*/
     return glm::mat4();
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

    std::vector<JointFramePose> JointPoses;


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

glm::mat4 Skeleton::GetBoneTransform(bool noRootMotion, const Bone* bone, std::vector<AnimationData> animations, AnimationOffset animationOffset, glm::mat4 childMatrix)
{
    glm::mat4 boneMatrix;

    std::vector<JointFramePose> JointPoses;

    for (const AnimationData animationData : animations) {
        const Animation* animation = animationData.animation;
        const float time = animationData.time;

        JointFramePose jointPose;
        jointPose.Weight = animationData.weight;;

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

                jointPose.Pose = (glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale));
                JointPoses.push_back(jointPose);

            } else { // 1 keyframes for the current bone
                currentFrame = boneKeyFrames.at(0);
                jointPose.Pose = (glm::translate(currentFrame.BoneProperties.Position) * glm::toMat4(currentFrame.BoneProperties.Rotation) * glm::scale(currentFrame.BoneProperties.Scale));
                JointPoses.push_back(jointPose);
            }
        } else { // 0 keyframes for the current bone

        }

    }


    if (JointPoses.size() == 0) {
        if (bone->Parent) {

            glm::mat4 jointPose = (glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix);
            glm::mat4 boneTransform = AdditiveBlend(bone, animationOffset, jointPose);

            boneMatrix = boneTransform * childMatrix;
        } else {
            boneMatrix = glm::inverse(bone->OffsetMatrix) * childMatrix;
        }
    } else {

        float totalWeight = 0;

        for (JointFramePose jointFramePose : JointPoses) {
            totalWeight += jointFramePose.Weight;
        }

        glm::mat4 finalBlend = glm::mat4(0);

        for (JointFramePose jointFramePose : JointPoses) {
            if (jointFramePose.Weight == 1.0f) {
                finalBlend = jointFramePose.Pose;
            } else {
                finalBlend += jointFramePose.Pose * (jointFramePose.Weight / totalWeight);
            }
        }


        glm::mat4 boneTransform = AdditiveBlend(bone, animationOffset, finalBlend);
        boneMatrix = boneTransform * childMatrix;
    }

    if (bone->Parent != nullptr) {
        return GetBoneTransform(noRootMotion, bone->Parent, animations, animationOffset, boneMatrix);
    } else {
        return boneMatrix;
    }
}

glm::mat4 Skeleton::GetBoneTransform(bool noRootMotion, const Bone* bone, std::vector<AnimationData> animations, glm::mat4 childMatrix)
{
    glm::mat4 boneMatrix;
   /* std::vector<JointFrameTransform> JointTransforms;

    for (const AnimationData animationData : animations) {
        const Animation* animation = animationData.animation;
        const float time = animationData.time;

        JointFrameTransform jointTransform;
        jointTransform.Weight = animationData.weight;;

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

                jointTransform.Position = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
                jointTransform.Rotation = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
                jointTransform.Scale = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

                // Flag for no root motion
                if (bone == RootBone && noRootMotion) {
                    jointTransform.Position.x = 0;
                    jointTransform.Position.z = 0;
                }

                JointTransforms.push_back(jointTransform);

            } else { // 1 keyframes for the current bone
                currentFrame = boneKeyFrames.at(0);
                jointTransform.Position = currentFrame.BoneProperties.Position;
                jointTransform.Rotation = currentFrame.BoneProperties.Rotation;
                jointTransform.Scale = currentFrame.BoneProperties.Scale;
                JointTransforms.push_back(jointTransform);

            }
        } else { // 0 keyframes for the current bone

        }

    }

    if (JointTransforms.size() <= 0) {
        if (bone->Parent) {
            boneMatrix = glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix * childMatrix;
        } else {
            boneMatrix = glm::inverse(bone->OffsetMatrix) * childMatrix;
        }
    } else if (JointTransforms.size() == 1) {
        boneMatrix = (glm::translate(JointTransforms.at(0).Position) * glm::toMat4(JointTransforms.at(0).Rotation) * glm::scale(JointTransforms.at(0).Scale)) * childMatrix;
    } else {

        glm::vec3 finalPosInterp;
        glm::quat finalRotInterp;
        glm::vec3 finalScaleInterp;
        float totalWeight = 0;

        for (JointFrameTransform jointTransform : JointTransforms) {
            totalWeight += jointTransform.Weight;
        }


        for (JointFrameTransform jointTransform : JointTransforms) {
            if (jointTransform.Weight == 1.0f) {
                finalPosInterp = jointTransform.Position;
                finalRotInterp = jointTransform.Rotation;
                finalScaleInterp = jointTransform.Scale;
                break;
            } else {
                finalPosInterp += jointTransform.Position * (jointTransform.Weight/totalWeight);
                finalRotInterp *= glm::slerp(glm::quat(), jointTransform.Rotation, (jointTransform.Weight/totalWeight));
                finalScaleInterp += jointTransform.Scale * (jointTransform.Weight/totalWeight);
            }

        }

        boneMatrix = (glm::translate(finalPosInterp) * glm::toMat4(finalRotInterp) * glm::scale(finalScaleInterp)) * childMatrix;
    }


    if (bone->Parent != nullptr) {
        return GetBoneTransform(noRootMotion, bone->Parent, animations, boneMatrix);
    } else {
        return boneMatrix;
    }*/

return boneMatrix;
}


std::map<int, glm::mat4> Skeleton::BlendPoses(std::map<int, glm::mat4> pose1, std::map<int, glm::mat4> pose2, float weight)
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


std::map<int, glm::mat4> Skeleton::OverridePose(std::map<int, glm::mat4> overridePose, std::map<int, glm::mat4> targetPose)
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


std::map<int, glm::mat4> Skeleton::BlendPoseAdditive(std::map<int, glm::mat4> additivePose, std::map<int, glm::mat4> targetPose)
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

std::vector<glm::mat4> Skeleton::GetFinalPose(std::map<int, glm::mat4>& boneMatrices)
{
    std::vector<glm::mat4> finalPose;

    AccumulateFinalPose(boneMatrices, RootBone, glm::mat4(1));


    for(auto& b : boneMatrices) {
        finalPose.push_back(b.second);
    }

    return finalPose;
}

void Skeleton::AccumulateFinalPose(std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix)
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

    for (auto &child : bone->Children) {
        AccumulateFinalPose(boneMatrices, child, boneMatrix);
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
