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

	Skeleton() { }
	~Skeleton();

	Bone* RootBone;

	std::map<int, Bone*> Bones;

	// Attach a new bone to the skeleton
	// Returns: New bone index
	int CreateBone(int ID, int parentID, std::string name, glm::mat4 offsetMatrix);
	int GetBoneID(std::string name);
    const Animation* GetAnimation(std::string name);

    std::map<int, glm::mat4> GetFrameBones(const Animation* animation, double time, bool additive, bool noRootMotion = false);
    glm::mat4 GetBoneTransform(const Bone* bone, const Animation* animation, float time, glm::mat4 childMatrix);
    std::map<int, glm::mat4> BlendPoses(const std::map<int, glm::mat4>& pose1, const std::map<int, glm::mat4>& pose2, float weight);
    std::map<int, glm::mat4> OverridePose(const std::map<int, glm::mat4>& overridePose, const std::map<int, glm::mat4>& targetPose);
    std::map<int, glm::mat4> BlendPoseAdditive(const std::map<int, glm::mat4>& additivePose, const std::map<int, glm::mat4>& targetPose);
    std::vector<glm::mat4> GetFinalPose(std::map<int, glm::mat4>& boneMatrices);
    
    std::map<std::string, Animation> Animations;
private:
    glm::mat4 GetAdditiveBonePose(const Bone* bone, const Animation* animation, double time);
    glm::mat4 GetBonePose(const Bone* bone, const Animation* animation, double time, bool noRootMotion);
    void AccumulateFinalPose(std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix);
    void AdditiveBoneTransforms(const Animation* animation, double time, std::map<int, glm::mat4>& boneMatrices, const Bone* bone);
    void AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, double time, std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix);

	std::map<std::string, Bone*> m_BonesByName;
    
};

#endif
