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

std::vector<glm::mat4> Skeleton::GetFrameBones(std::vector<AnimationData> animations, bool noRootMotion /*= false*/)
{
    if (animations.size() <= 0) {
        std::vector<glm::mat4> finalMatrices;
        for (auto& b : Bones) {
            finalMatrices.push_back(glm::mat4(1));//b.second->OffsetMatrix);
        }
        return finalMatrices;
    }

    std::map<int, glm::mat4> frameBones;
    AccumulateBoneTransforms(true, animations, frameBones, RootBone, glm::mat4(1));

    std::vector<glm::mat4> finalMatrices;
    for (auto &kv : frameBones) {
        finalMatrices.push_back(kv.second);
    }
    return finalMatrices;
}


std::vector<glm::mat4> Skeleton::GetFrameBones(std::vector<AnimationData> animations, AnimationOffset animationOffset, bool noRootMotion /*= false*/)
{
    if (animations.size() <= 0 || animationOffset.animation == nullptr) {
        std::vector<glm::mat4> finalMatrices;
        for (auto& b : Bones) {
            finalMatrices.push_back(glm::mat4(1));//b.second->OffsetMatrix);
        }
        return finalMatrices;
    }


    std::map<int, glm::mat4> frameBones;
    AccumulateBoneTransforms(true, animations, animationOffset, frameBones, RootBone, glm::mat4(1));

    std::vector<glm::mat4> finalMatrices;
    for (auto &kv : frameBones) {
        finalMatrices.push_back(kv.second);
    }
    return finalMatrices;
}

void Skeleton::AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, float time, std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix)
{
    glm::mat4 boneMatrix;

    Animation::Keyframe currentFrame;
    Animation::Keyframe nextFrame;

    if(animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) { 
        std::vector<Animation::Keyframe> boneKeyFrames = animation->JointAnimations.at(bone->ID);

        if(boneKeyFrames.size() > 1) { // 2+ keyframes for the current bone
            for (int index = boneKeyFrames.size()-1; index >= 0; index--) { // find the bone keyframes that surrounds the current frame
                if (time >= boneKeyFrames.at(index).Time) {
                    currentFrame = boneKeyFrames.at(index);
                    nextFrame = boneKeyFrames.at((index + 1) % boneKeyFrames.size());
                    break;
                }
            }

            float progress;

            if(nextFrame.Index == 0) {
                progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
            } else {
                progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);

            }


            if (progress > 1.0f || progress < 0.0f) {
                LOG_INFO("Progress: %f", progress);
                progress = glm::clamp(progress, 0.0f, 1.0f);
            }
            Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties;
            Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties;

            glm::vec3 positionInterp = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
            glm::quat rotationInterp = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
            glm::vec3 scaleInterp = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

            // Flag for no root motion
            if (bone == RootBone && noRootMotion) {
                positionInterp.x = 0;
                positionInterp.z = 0;
            }

            boneMatrix =  parentMatrix *(glm::translate(positionInterp) * glm::toMat4(rotationInterp) * glm::scale(scaleInterp));
            boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;

        } else { // 1 keyframes for the current bone
            currentFrame = boneKeyFrames.at(0);
            boneMatrix = parentMatrix *(glm::translate(currentFrame.BoneProperties.Position) * glm::toMat4(currentFrame.BoneProperties.Rotation) * glm::scale(currentFrame.BoneProperties.Scale));

            boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
        }
    } else { // 0 keyframes for the current bone

     //   LOG_INFO("%s Has no keyframe", bone->Name.c_str());
        if (bone->Parent) {
            boneMatrix = parentMatrix * glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix;
            boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
        } else {
            boneMatrix = glm::inverse(bone->OffsetMatrix);
            boneMatrices[bone->ID] = parentMatrix;
        }
        
    }

    for (auto &child : bone->Children) {
        AccumulateBoneTransforms(noRootMotion, animation, time, boneMatrices, child, boneMatrix);
    }  
}

void Skeleton::AccumulateBoneTransforms(bool noRootMotion, std::vector<AnimationData> animations, std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix)
{
    glm::mat4 boneMatrix;



    std::vector<JointFrameTransform> JointTransforms;

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
                    progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
                } else {
                    progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);

                }


                if (progress > 1.0f || progress < 0.0f) {
                    LOG_INFO("Progress: %f", progress);
                    progress = glm::clamp(progress, 0.0f, 1.0f);
                }
                Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties;
                Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties;

                jointTransform.PositionInterp = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
                jointTransform.RotationInterp = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
                jointTransform.ScaleInterp = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

                // Flag for no root motion
                if (bone == RootBone && noRootMotion) {
                    jointTransform.PositionInterp.x = 0;
                    jointTransform.PositionInterp.z = 0;
                }

                JointTransforms.push_back(jointTransform);

            } else { // 1 keyframes for the current bone
                currentFrame = boneKeyFrames.at(0);
                jointTransform.PositionInterp = currentFrame.BoneProperties.Position;
                jointTransform.RotationInterp = currentFrame.BoneProperties.Rotation;
                jointTransform.ScaleInterp = currentFrame.BoneProperties.Scale;
                JointTransforms.push_back(jointTransform);

            }
        } else { // 0 keyframes for the current bone
          
        }

    }

    if(JointTransforms.size() <= 0) {
        if (bone->Parent) {
            boneMatrix = parentMatrix * glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix;
            boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
        } else {
            boneMatrix = glm::inverse(bone->OffsetMatrix);
            boneMatrices[bone->ID] = parentMatrix;
        }
    } else if (JointTransforms.size() == 1) {
        boneMatrix = parentMatrix *(glm::translate(JointTransforms.at(0).PositionInterp) * glm::toMat4(JointTransforms.at(0).RotationInterp) * glm::scale(JointTransforms.at(0).ScaleInterp));
        boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
    } else {

        glm::vec3 finalPosInterp;
        glm::quat finalRotInterp;
        glm::vec3 finalScaleInterp;
        float totalWeight = 0;

        for (JointFrameTransform jointTransform : JointTransforms) {
            totalWeight += jointTransform.Weight;
        }


        for (JointFrameTransform jointTransform : JointTransforms)
        {
            if(jointTransform.Weight == 1.0f) {
                finalPosInterp = jointTransform.PositionInterp;
                finalRotInterp = jointTransform.RotationInterp;
                finalScaleInterp = jointTransform.ScaleInterp;
                break;
            } else {
                finalPosInterp += jointTransform.PositionInterp * (jointTransform.Weight/totalWeight);
                finalRotInterp *= glm::slerp(glm::quat(), jointTransform.RotationInterp, (jointTransform.Weight/totalWeight));
                finalScaleInterp += jointTransform.ScaleInterp * (jointTransform.Weight/totalWeight);
            }
            
        }

        boneMatrix = parentMatrix *(glm::translate(finalPosInterp) * glm::toMat4(finalRotInterp) * glm::scale(finalScaleInterp));
        boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
    }



    for (auto &child : bone->Children) {
        AccumulateBoneTransforms(noRootMotion, animations, boneMatrices, child, boneMatrix);
    }
}


void Skeleton::AccumulateBoneTransforms(bool noRootMotion, std::vector<AnimationData> animations, AnimationOffset animationOffset, std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix)
{
    glm::mat4 boneMatrix;



    std::vector<JointFrameTransform> JointTransforms;

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
                    progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
                } else {
                    progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);

                }


                if (progress > 1.0f || progress < 0.0f) {
                    LOG_INFO("Progress: %f", progress);
                    progress = glm::clamp(progress, 0.0f, 1.0f);
                }
                Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties;
                Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties;

                jointTransform.PositionInterp = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
                jointTransform.RotationInterp = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
                jointTransform.ScaleInterp = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;

                // Flag for no root motion
                if (bone == RootBone && noRootMotion) {
                    jointTransform.PositionInterp.x = 0;
                    jointTransform.PositionInterp.z = 0;
                }

                JointTransforms.push_back(jointTransform);

            } else { // 1 keyframes for the current bone
                currentFrame = boneKeyFrames.at(0);
                jointTransform.PositionInterp = currentFrame.BoneProperties.Position;
                jointTransform.RotationInterp = currentFrame.BoneProperties.Rotation;
                jointTransform.ScaleInterp = currentFrame.BoneProperties.Scale;
                JointTransforms.push_back(jointTransform);

            }
        } else { // 0 keyframes for the current bone

        }

    }


    glm::mat4 offset = GetOffsetTransform(bone, animationOffset);

    if (JointTransforms.size() == 0) {
        if (bone->Parent) {
            if (offset != glm::mat4(1)) {
                boneMatrix = parentMatrix * offset;// *((glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix));
            } else {
                boneMatrix = parentMatrix  *((glm::inverse(bone->OffsetMatrix) * bone->Parent->OffsetMatrix));

            }
            boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
        } else {
            boneMatrix = offset * glm::inverse(bone->OffsetMatrix);
            boneMatrices[bone->ID] = parentMatrix;
        }
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
                finalPosInterp = jointTransform.PositionInterp;
                finalRotInterp = jointTransform.RotationInterp;
                finalScaleInterp = jointTransform.ScaleInterp;
                break;
            } else {
                finalPosInterp += jointTransform.PositionInterp * (jointTransform.Weight/totalWeight);
                finalRotInterp *= glm::slerp(glm::quat(), jointTransform.RotationInterp, (jointTransform.Weight/totalWeight));
                finalScaleInterp += jointTransform.ScaleInterp * (jointTransform.Weight/totalWeight);
            }

        }



        if (offset != glm::mat4(1)) {
            boneMatrix = parentMatrix * ((glm::translate(finalPosInterp) * glm::toMat4(finalRotInterp) * glm::scale(finalScaleInterp)) + offset);
        } else {
            boneMatrix = parentMatrix * (glm::translate(finalPosInterp) * glm::toMat4(finalRotInterp) * glm::scale(finalScaleInterp));
        }
        
        boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;
    }

    for (auto &child : bone->Children) {
        AccumulateBoneTransforms(noRootMotion, animations, animationOffset, boneMatrices, child, boneMatrix);
    }
}


glm::mat4 Skeleton::GetOffsetTransform(const Bone* bone, AnimationOffset animationOffset)
{
    const Animation* animation = animationOffset.animation;
    float time = animationOffset.time;

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
                progress = (time - currentFrame.Time) / (animation->Duration - currentFrame.Time);
            } else {
                progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time);

            }


            if (progress > 1.0f || progress < 0.0f) {
                LOG_INFO("Progress: %f", progress);
                progress = glm::clamp(progress, 0.0f, 1.0f);
            }
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

    return (glm::translate(position) * glm::toMat4(rotation) * glm::scale(scale));
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

int Skeleton::GetBoneID(std::string name)
{
	if (m_BonesByName.find(name) == m_BonesByName.end()) {
		return -1;
	} else {
		return m_BonesByName.at(name)->ID;
	}
}

void Skeleton::PrintSkeleton()
{
	if (LOG_LEVEL < LOG_LEVEL_DEBUG) {
		return;
	}
	PrintSkeleton(RootBone, 0);
}

void Skeleton::PrintSkeleton(const Bone* bone, int depthCount)
{
	std::stringstream ss;
	ss << std::string(depthCount, ' ');
	ss << bone->ID << ": " << bone->Name;
	std::cout << ss.str() << std::endl;
	
	depthCount++;

	for (auto &child : bone->Children) {
		PrintSkeleton(child, depthCount);
	}
}

int Skeleton::GetKeyframe(const Animation& animation, double time)
{

/*
	if (time < 0) {
		time = 0;
	}
	if (time >= animation.Duration) {
		return animation..size() - 1;
	}

	for (int keyframe = 0; keyframe < animation.Keyframes.size(); ++keyframe) {
		if (animation.Keyframes[keyframe].Time > time) {
			return (keyframe - 1) % animation.Keyframes.size();
		}
	}
*/


	return 0;
}
