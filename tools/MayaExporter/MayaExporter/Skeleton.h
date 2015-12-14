#ifndef Skeleton_Skeleton_h__
#define Skeleton_Skeleton_h__

#include <vector>
#include <array>
#include "MayaIncludes.h"

struct Joint {
	int ParentIndex;
	std::array<float, 3> Rotation;
	std::array<float, 3> Translation;
	std::array<float, 3> Scale;
};

struct SkeletonNode {
	std::string Name;
	std::vector<Joint> joints;
};

class Skeleton {
public:
	//std::vector<SkeletonNode>* DoIt();
private:
	//std::vector<SkeletonNode> m_AllSkeletons;
};

#endif //Skeleton_Skeleton_h__