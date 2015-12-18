#ifndef Skeleton_Skeleton_h__
#define Skeleton_Skeleton_h__

#include <vector>
#include <array>
#include <algorithm>
#include "MayaIncludes.h"
#include "OutputData.h"
class Joint : public OutputData{
public:
	//Joint() : OutputData((Joint)*this)
	//{};
	//int ParentIndex;
	//std::array<float, 3> Rotation;
	//std::array<float, 3> Translation;
	//std::array<float, 3> Scale;
	//std::array<std::array<float, 4>, 4> OffsetMatrix;
	float OffsetMatrix[4][4];

	virtual void WriteBinary(std::ostream& out)
	{
		out.write((char*)OffsetMatrix, 4 * 4 * sizeof(float));
	}

	virtual void WriteASCII(std::ostream& out) const
	{
		for (int i = 0; i < 4; i++)
		{
			for (int k = 0; k < 4; k++)
				out << this->OffsetMatrix[i][k] << " ";
			out << endl;
		}
	};
};

class SkeletonNode : public OutputData {
public:
	std::string Name;
	std::vector<Joint> Joints;

	virtual void WriteBinary(std::ostream& out)
	{
		out.write(Name.c_str(), Name.size());
		for (auto aJoint : Joints)
		{
			aJoint.WriteBinary(out);
		}
	}
	virtual void WriteASCII(std::ostream& out) const
	{
		out << "Skeleton: " << Name << endl;
		for (auto aJoint : Joints)
		{
			aJoint.WriteASCII(out);
		}
	};
};

class BindPoseSkeletonNode : public SkeletonNode {
public:
	std::vector<int> ParentIDs;
	std::vector<std::string> JointNames;
	virtual void WriteBinary(std::ostream& out)
	{
		out.write(Name.c_str(), Name.size());
		for (int i = 0; i < Joints.size(); i++)
		{
			out.write((char*)&ParentIDs[i],sizeof(int));
			Joints[i].WriteBinary(out);
			out.write(JointNames[i].c_str(), JointNames[i].size());
		}

	}

	virtual void WriteASCII(std::ostream& out) const
	{
		out << "Bind Pose: " << Name << endl;
		for (int i = 0; i < Joints.size(); i++)
		{
			out << ParentIDs[i] << endl;
			Joints[i].WriteASCII(out);
			out << JointNames[i] << endl;
		}
	};
};

class Skeleton {
public:
	std::vector<SkeletonNode> DoIt();
	std::vector<BindPoseSkeletonNode> GetBindPoses();
private:
};

#endif //Skeleton_Skeleton_h__