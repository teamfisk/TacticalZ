#ifndef Skeleton_Skeleton_h__
#define Skeleton_Skeleton_h__

#include <vector>
#include <array>
#include <algorithm>
#include "MayaIncludes.h"

struct Joint {
	int ParentIndex;
	//std::array<float, 3> Rotation;
	//std::array<float, 3> Translation;
	//std::array<float, 3> Scale;
	std::array<std::array<float, 4>, 4> OffsetMatrix;
	std::string Name;
};

struct SkeletonNode {
	std::string Name;
	std::vector<Joint> Joints;
};

class Skeleton {
public:
	std::vector<SkeletonNode>* DoIt();
private:
	std::vector<SkeletonNode> m_AllSkeletons;
	std::vector<MObject> m_Hierarchy;
};

#endif //Skeleton_Skeleton_h__