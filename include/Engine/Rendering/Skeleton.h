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

    struct AnimationData
    {
        const Animation* animation;
        float time;
        float weight;
    };

    struct JointFrameTransform {
        glm::vec3 PositionInterp = glm::vec3(0);
        glm::quat RotationInterp = glm::quat();
        glm::vec3 ScaleInterp = glm::vec3(0);
        float Weight;
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

    std::vector<glm::mat4>  GetFrameBones(std::vector<AnimationData> animations, AnimationOffset animationOffset, bool noRootMotion = false);
    std::vector<glm::mat4>  GetFrameBones(std::vector<AnimationData> animations, bool noRootMotion = false);

    const Animation* GetAnimation(std::string name);

    void AccumulateBoneTransforms(bool noRootMotion, std::vector<AnimationData> animations, std::map<int, glm::mat4>& frameBones, const Bone* bone, glm::mat4 parentMatrix);
    void AccumulateBoneTransforms(bool noRootMotion, std::vector<AnimationData> animations, AnimationOffset animationOffset, std::map<int, glm::mat4>& frameBones, const Bone* bone, glm::mat4 parentMatrix);

    glm::mat4 AdditiveBlend(JointFrameTransform addTransform, JointFrameTransform transform);

    void PrintSkeleton();
	void PrintSkeleton(const Bone* parent, int depthCount);
	std::map<std::string, Animation> Animations;

    glm::mat4 GetBoneTransform(const Bone* bone, const Animation* animation, float time, glm::mat4 childMatrix);
    glm::mat4 GetBoneTransform(bool noRootMotion, const Bone* bone, std::vector<AnimationData> animations, AnimationOffset animationOffset, glm::mat4 childMatrix);
    glm::mat4 GetBoneTransform(bool noRootMotion, const Bone* bone, std::vector<AnimationData> animations, glm::mat4 childMatrix);
    int GetKeyframe(const Animation& animation, double time);

private:
    JointFrameTransform GetOffsetTransform(const Bone* bone, AnimationOffset animationOffset);

	std::map<std::string, Bone*> m_BonesByName;
    
};

#endif
