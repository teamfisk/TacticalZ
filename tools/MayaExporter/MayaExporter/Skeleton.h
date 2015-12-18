#ifndef Skeleton_Skeleton_h__
#define Skeleton_Skeleton_h__

#include <vector>
#include <array>
#include <algorithm>
#include "MayaIncludes.h"

struct Joint {
	//int ParentIndex;
	//std::array<float, 3> Rotation;
	//std::array<float, 3> Translation;
	//std::array<float, 3> Scale;
	std::array<std::array<float, 4>, 4> OffsetMatrix;
};

struct SkeletonNode {
	std::string Name;
	std::vector<Joint> Joints;
};

struct BindPoseSkeletonNode : SkeletonNode {
	std::vector<int> ParentIDs;
	std::vector<std::string> JointNames;
};

class Skeleton {
public:
	std::vector<SkeletonNode> DoIt();
	std::vector<BindPoseSkeletonNode> GetBindPoses();
private:
};

#endif //Skeleton_Skeleton_h__