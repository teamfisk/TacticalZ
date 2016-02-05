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

std::vector<glm::mat4> Skeleton::GetFrameBones(const Animation* animation, double time, bool noRootMotion /*= false*/)
{
    if(animation == nullptr) {
        std::vector<glm::mat4> finalMatrices;
        for(auto& b : Bones) {
            finalMatrices.push_back(b.second->ModificationMatrix);//b.second->OffsetMatrix);
        }
        return finalMatrices;
    }


	// HACK: Animation wrap-around

	while (time < 0) {
		time += animation->Duration;
	}
	while (time > animation->Duration) {
		time -= animation->Duration;
	}

	//auto animationFrame = Animations[""].Keyframes[frame];
	std::map<int, glm::mat4> frameBones;
    AccumulateBoneTransforms(noRootMotion, animation, time, frameBones, RootBone, glm::mat4(1));
    //AccumulateBoneTransforms(noRootMotion, currentFrame, nextFrame, alpha, frameBones, RootBone, glm::mat4(1));

	std::vector<glm::mat4> finalMatrices;
	for (auto &kv : frameBones) {
		finalMatrices.push_back(kv.second);
	}
	return finalMatrices;
}




void Skeleton::AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, float time, std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix)
{
    glm::mat4 boneMatrix;// = glm::mat4(1);

    Animation::Keyframe currentFrame;
    Animation::Keyframe nextFrame;

    if(animation->JointAnimations.find(bone->ID) != animation->JointAnimations.end()) { // find the bone keyframes that surrounds the current frame
        std::vector<Animation::Keyframe> boneKeyFrames = animation->JointAnimations.at(bone->ID);
   
        if(boneKeyFrames.size() > 1) { // 2+ keyframes for the current bone
            for (int index = boneKeyFrames.size()-1; index >= 0; index--) {
                if (time >= boneKeyFrames.at(index).Time) {
                    currentFrame = boneKeyFrames.at(index);
                    nextFrame = boneKeyFrames.at((index + 1) % boneKeyFrames.size());
                    break;
                }
            }

            float progress = (time - currentFrame.Time) / (nextFrame.Time - currentFrame.Time); // fix loopinguuuu
            if(progress > 1.0f || progress < 0.0f) {
                LOG_INFO("Progress %f", progress);
                progress = 0.f;
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

            boneMatrix = parentMatrix * bone->ModificationMatrix * (glm::translate(positionInterp) * glm::toMat4(rotationInterp) * glm::scale(scaleInterp));
            boneMatrices[bone->ID] = boneMatrix *bone->OffsetMatrix;

        } else { // 1 keyframes for the current bone
            currentFrame = boneKeyFrames.at(0);
            boneMatrix = parentMatrix * bone->ModificationMatrix *(glm::translate(currentFrame.BoneProperties.Position) * glm::toMat4(currentFrame.BoneProperties.Rotation) * glm::scale(currentFrame.BoneProperties.Scale));
            boneMatrices[bone->ID] = boneMatrix *bone->OffsetMatrix;
        }
    } else { // 0 keyframes for the current bone
        boneMatrix = parentMatrix * bone->ModificationMatrix;
        boneMatrices[bone->ID] = boneMatrix * bone->OffsetMatrix;


    }

    for (auto &child : bone->Children) {
        std::string name = child->Name;
        AccumulateBoneTransforms(noRootMotion, animation, time, boneMatrices, child, boneMatrix);
    }  
}

glm::mat4 Skeleton::GetBoneTransform(const Bone* bone, const Animation::Keyframe& currentFrame, const Animation::Keyframe& nextFrame, float progress, glm::mat4 parentMatrix)
{
   /* glm::mat4 boneMatrix;

    if (currentFrame.BoneProperties.find(bone->ID) != currentFrame.BoneProperties.end() || nextFrame.BoneProperties.find(bone->ID) != nextFrame.BoneProperties.end()) {
        Animation::Keyframe::BoneProperty currentBoneProperty = currentFrame.BoneProperties.at(bone->ID);
        Animation::Keyframe::BoneProperty nextBoneProperty = nextFrame.BoneProperties.at(bone->ID);

        glm::vec3 positionInterp = currentBoneProperty.Position * (1.f - progress) + nextBoneProperty.Position * progress;
        glm::quat rotationInterp = glm::slerp(currentBoneProperty.Rotation, nextBoneProperty.Rotation, progress);
        glm::vec3 scaleInterp = currentBoneProperty.Scale * (1.f - progress) + nextBoneProperty.Scale * progress;


        boneMatrix = (glm::translate(positionInterp) * glm::toMat4(rotationInterp) * glm::scale(scaleInterp)) * bone->ModificationMatrix * parentMatrix;
    } else {
        if (bone->Parent) {
            boneMatrix = parentMatrix;
        }
    }

    if(bone->Parent) {
        return  GetBoneTransform(bone->Parent, currentFrame, nextFrame, progress, boneMatrix);
    } else {
        return boneMatrix;
    }*/
    return parentMatrix;
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
