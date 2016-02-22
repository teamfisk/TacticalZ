#ifndef Skeleton_h__
#define Skeleton_h__

#include <sstream>
#include "Common.h"
#include "../GLM.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui/imgui.h>

//struct Bone
//{
//	Bone(std::string name, glm::mat4 offsetMatrix)
//		: Name(name)
//		, OffsetMatrix(offsetMatrix)
//	{ }
//
//	~Bone()
//	{
//		for (auto kv : Children) {
//			delete kv.second;
//		}
//	}
//
//	std::string Name;
//	glm::mat4 OffsetMatrix;
//	glm::mat4 LocalMatrix;
//
//	std::map<std::string, Bone*> Children;
//};

class Skeleton
{
public:
	struct Bone
	{
		Bone(int id, Bone* parent, std::string name, glm::mat4 offsetMatrix)
			: ID(id)
			, Parent(parent)
			, Name(name)
			, OffsetMatrix(offsetMatrix)
		{ }

		std::string Name;
		glm::mat4 OffsetMatrix;
        int ID;

		Bone* Parent;
		std::vector<Bone*> Children;
	};

	struct Animation
	{
		struct Keyframe
		{
			struct BoneProperty
			{
                glm::vec3 Position;
                glm::quat Rotation;
				glm::vec3 Scale = glm::vec3(1);
			};

            int Index = 0;
            double Time = 0.0;
            BoneProperty BoneProperties;
		};
        std::string Name;
        double Duration;
        std::map<int, std::vector<Keyframe>> JointAnimations;
	};

    enum class BlendType
    {
        Additive,
        Blend,
        Override,
    };

    struct AnimationData
    {
        const Animation* animation;
        BlendType blendType;
        float time;
        int level;
        float weight;
    };

    struct JointFramePose {
        BlendType Type;
        int Level = 0;
        glm::mat4 Pose = glm::mat4(0);
        float Weight = 0.0f;
    };

    struct AnimationOffset {
        const Animation* animation;
        float time;
    };

	Skeleton() { }
	~Skeleton();

	Bone* RootBone;

	std::map<int, Bone*> Bones;

	// Attach a new bone to the skeleton
	// Returns: New bone index
	int CreateBone(int ID, int parentID, std::string name, glm::mat4 offsetMatrix);

	int GetBoneID(std::string name);

    std::vector<glm::mat4>  GetFrameBones();

    std::vector<glm::mat4> GetFrameBones(const Animation* animation, double time, bool additive, bool noRootMotion = false);
    void AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, double time, std::map<int, glm::mat4>& boneMatrices, bool additive, const Bone* bone, glm::mat4 parentMatrix);

    const Animation* GetAnimation(std::string name);
    glm::mat4 AdditiveBlend(const Bone* bone, AnimationOffset animationOffset, glm::mat4 targetPose);

	std::map<std::string, Animation> Animations;

    glm::mat4 GetBoneTransform(const Bone* bone, const Animation* animation, float time, glm::mat4 childMatrix);
    glm::mat4 GetBoneTransform(bool noRootMotion, const Bone* bone, std::vector<AnimationData> animations, AnimationOffset animationOffset, glm::mat4 childMatrix);
    glm::mat4 GetBoneTransform(bool noRootMotion, const Bone* bone, std::vector<AnimationData> animations, glm::mat4 childMatrix);


    std::vector<glm::mat4> BlendPoses(std::vector<glm::mat4> pose1, std::vector<glm::mat4> pose2, float weight);
    std::vector<glm::mat4> OverridePose(std::vector<glm::mat4> overridePose, std::vector<glm::mat4> targetPose);

private:
    glm::mat4 GetOffsetTransform(const Bone* bone, AnimationOffset animationOffset);
    glm::mat4 GetBonePose(const Bone* bone, const Animation* animation, double time, bool noRootMotion);

	std::map<std::string, Bone*> m_BonesByName;
    
};

#endif
