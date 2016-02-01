#ifndef Skeleton_Skeleton_h__
#define Skeleton_Skeleton_h__

#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include "MayaIncludes.h"
#include "OutputData.h"

class Animation : public OutputData {
public:
	struct Keyframe
	{
		struct JointProperty
		{
			int ID = 0;
            float Position[3]{ 0 };
            float Rotation[4]{ 0 };
            float Scale[3]{ 0 };
		};

		int Index = 0;
		float Time = 0;
		std::vector<JointProperty> JointProperties;
	};

	std::string Name;
    int nameLength = 0;
	float Duration = 0;
    int NumKeyFrames = 0;
    int NumberOfJoints = 0;
	std::vector<Keyframe> Keyframes;

	virtual void WriteBinary(std::ostream& out)
	{
        out.write((char*)&nameLength, sizeof(int));
		out.write(Name.c_str(), Name.size() + 1);
		out.write((char*)&Duration, sizeof(float));
        out.write((char*)&NumKeyFrames, sizeof(int));
        out.write((char*)&NumberOfJoints, sizeof(int));
        //Här under loopas alla key frames igenom
		for (auto aKeyframe : Keyframes) {
			out.write((char*)&aKeyframe.Index, sizeof(int));
			out.write((char*)&aKeyframe.Time, sizeof(float));
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
        out << "Number of KeyFrames: " << NumKeyFrames << endl;
        out << "Number of Joints: " << NumberOfJoints << endl;
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
        int NameLength;
		std::string Name;
        float OffsetMatrix[4][4]{ 0 };
        int ID = 0;
        int ParentID = 0;
	};
    int numBones = 0;
	std::string Name;
	std::vector<BindPoseJoint> Joints;
	virtual void WriteBinary(std::ostream& out)
	{
        out.write((char*)&numBones, sizeof(int));
		for (auto Joint:Joints)
		{	
            out.write((char*)&Joint.NameLength, sizeof(int));
			out.write(Joint.Name.c_str(), Joint.Name.size() + 1);
			out.write((char*)&Joint.OffsetMatrix, sizeof(float) * 4 * 4);
            out.write((char*)&Joint.ID, sizeof(int));
			out.write((char*)&Joint.ParentID, sizeof(int));
		}

	}

	virtual void WriteASCII(std::ostream& out) const
	{
		out << "Bind Pose: " << Name << " _ not in binary" << endl;
        out << "numberOfBones: " << numBones << endl;
		for (auto Joint : Joints)
		{
            out << "Joint NameLength " << Joint.NameLength << endl;
			out << "Joint name " << Joint.Name << endl;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++){
					out << Joint.OffsetMatrix[i][j] << " ";
				}
				out << endl;
			}
            out << Joint.ID << endl;
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