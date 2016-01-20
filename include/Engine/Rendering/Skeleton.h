#ifndef Skeleton_h__
#define Skeleton_h__

#include <sstream>
#include "Common.h"
#include "../GLM.h"

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
				int ID;
				glm::vec3 Position;
				glm::quat Rotation;
				glm::vec3 Scale = glm::vec3(1);
			};

			int Index = 0;
			double Time = 0.0;
			std::map<int, Keyframe::BoneProperty> BoneProperties;
		};

		std::string Name;
		double Duration;
		std::vector<Keyframe> Keyframes;
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
	std::vector<glm::mat4> GetFrameBones(const Animation& animation, double time, bool noRootMotion = false);
	void AccumulateBoneTransforms(bool noRootMotion, const Animation::Keyframe& currentFrame, const Animation::Keyframe& nextFrame, float progress, std::map<int, glm::mat4>& boneMatrices, const Bone* bone, glm::mat4 parentMatrix);
	void PrintSkeleton();
	void PrintSkeleton(const Bone* parent, int depthCount);
	std::map<std::string, Animation> Animations;

private:
	std::map<std::string, Bone*> m_BonesByName;

	int GetKeyframe(const Animation& animation, double time);
};

#endif
