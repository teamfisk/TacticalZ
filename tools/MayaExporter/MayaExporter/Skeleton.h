#ifndef Skeleton_Skeleton_h__
#define Skeleton_Skeleton_h__

#include <vector>
#include <array>
#include <algorithm>
#include "MayaIncludes.h"
#include "OutputData.h"

class Animation : public OutputData {
public:
	struct Keyframe
	{
		struct JointProperty
		{
			int ID;
			float Position[3];
			float Rotation[4];
			float Scale[3];
		};

		int Index;
		double Time;
		std::vector<JointProperty> JointProperties;
	};

	std::string Name;
	double Duration;
	std::vector<Keyframe> Keyframes;

	virtual void WriteBinary(std::ostream& out)
	{
		out.write(Name.c_str(), Name.size() + 1);
		out.write((char*)&Duration, sizeof(double));
		for (auto aKeyframe : Keyframes) {
			out.write((char*)&aKeyframe.Index, sizeof(int));
			out.write((char*)&aKeyframe.Time, sizeof(double));
			for (auto aJoint : aKeyframe.JointProperties) {
				out.write((char*)&aJoint.ID, sizeof(int));
				out.write((char*)aJoint.Position, sizeof(float) * 3);
				out.write((char*)aJoint.Rotation, sizeof(float) * 4);
				out.write((char*)aJoint.Scale, sizeof(float) * 3);
			}
		}
	}

	virtual void WriteASCII(std::ostream& out) const
	{
		out << "Animation Name: " << Name << endl;
		out << "Duration: " << Duration << endl;
		for (auto aKeyframe : Keyframes) {
			out << "Frame: " << aKeyframe.Index << endl;
			out << "Time: " << aKeyframe.Time << endl;
			for (auto aJoint : aKeyframe.JointProperties) {
				out << "Joint ID: " << aJoint.ID << endl;
				out << aJoint.Position[0] << " " << aJoint.Position[1] << " " << aJoint.Position[2] << endl;
				out << aJoint.Rotation[0] << " " << aJoint.Rotation[1] << " " << aJoint.Rotation[2] << " " << aJoint.Rotation[3] << endl;
				out << aJoint.Scale[0] << " " << aJoint.Scale[1] << " " << aJoint.Scale[2] << endl;
			}
		}
		
	}
};

class BindPoseSkeletonNode : public OutputData {
public:
	struct BindPoseJoint
	{
		int ParentID;
		std::string Name;
		float OffsetMatrix[4][4];
	};

	std::string Name;
	std::vector<BindPoseJoint> Joints;
	virtual void WriteBinary(std::ostream& out)
	{
		out.write(Name.c_str(), Name.size() + 1);
		for (auto Joint:Joints)
		{	
			out.write(Joint.Name.c_str(), Joint.Name.size() + 1);
			out.write((char*)&Joint.OffsetMatrix, sizeof(float) * 4 * 4);
			out.write((char*)&Joint.ParentID, sizeof(int));
		}

	}

	virtual void WriteASCII(std::ostream& out) const
	{
		out << "Bind Pose: " << Name << endl;
		for (auto Joint : Joints)
		{
			out << Joint.Name << endl;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++){
					out << Joint.OffsetMatrix[i][j] << " ";
				}
				out << endl;
			}
			out << Joint.ParentID << endl;
		}
	};
};

class Skeleton {
public:
	//std::vector<SkeletonNode> DoIt();
	Animation GetAnimData(std::string animationName, int startFrame, int endFrame);
	std::vector<BindPoseSkeletonNode> GetBindPoses();
private:
};

#endif //Skeleton_Skeleton_h__