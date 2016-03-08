#ifndef Skeleton_h__
#define Skeleton_h__

#include <sstream>
#include "Common.h"
#include "../GLM.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <imgui/imgui.h>
#include "../Core/EntityWrapper.h"

class BlendTree;

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

    struct PoseData {
        glm::vec3 Translation;
        glm::quat Orientation;
        glm::vec3 Scale;
    };

	Skeleton() { }
	~Skeleton();

	Bone* RootBone;
	std::map<int, Bone*> Bones;

    std::unordered_map<EntityWrapper, std::shared_ptr<BlendTree>> BlendTrees;

	// Attach a new bone to the skeleton
	// Returns: New bone index
	int CreateBone(int ID, int parentID, std::string name, glm::mat4 offsetMatrix);
	int GetBoneID(std::string name);
    const Animation* GetAnimation(std::string name);

    std::map<int, Skeleton::PoseData> GetFrameBones(const Animation* animation, double time, bool additive, bool noRootMotion = false);
    
    std::map<int, Skeleton::PoseData> BlendPoses(const std::map<int, PoseData>& pose1, const std::map<int, PoseData>& pose2, double weight);
    std::map<int, Skeleton::PoseData> OverridePose(const std::map<int, PoseData>& overridePose, const std::map<int, PoseData>& targetPose);
    std::map<int, Skeleton::PoseData> BlendPoseAdditive(const std::map<int, PoseData>& additivePose, const std::map<int, PoseData>& targetPose);
    void GetFinalPose(std::map<int, Skeleton::PoseData>& boneMatrices, std::vector<glm::mat4>& finalPose, std::map<int, glm::mat4>& boneTransforms);
    
    std::vector<glm::mat4> GetTPose();


    std::map<std::string, Animation> Animations;
private:
    Skeleton::PoseData GetAdditiveBonePose(const Bone* bone, const Animation* animation, double time);
    
    void AccumulateFinalPose(std::map<int, glm::mat4>& boneMatrices, std::map<int, Skeleton::PoseData>& poseDatas, std::map<int, glm::mat4>& boneTransforms, const Bone* bone, glm::mat4 parentMatrix);
    void AdditiveBoneTransforms(const Animation* animation, double time, std::map<int, PoseData>& boneMatrices, const Bone* bone);
    void AccumulateBoneTransforms(bool noRootMotion, const Animation* animation, double time, std::map<int, PoseData>& boneMatrices, const Bone* bone);

	std::map<std::string, Bone*> m_BonesByName;
    
};

#endif
