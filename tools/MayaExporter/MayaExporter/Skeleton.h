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
	//struct Keyframe
	//{
	//	struct JointProperty
	//	{
	//		int ID = 0;
 //           float Position[3]{ 0 };
 //           float Rotation[4]{ 0 };
 //           float Scale[3]{ 0 };
	//	};

	//	int Index = 0;
	//	float Time = 0;
 //       int NumberOfJoints;
	//	std::vector<JointProperty> JointProperties;
	//};

    struct JointAnimation {
        struct KeyFrame {
            int Index = 0;
            float Time = 0;
            float Position[3]{ 0 };
            float Rotation[4]{ 0 };
            float Scale[3]{ 0 };
        };
        unsigned int numberOFKeyFrames = 0;
        std::vector<KeyFrame> m_KeyFrames;
    };

	std::string Name;
    int nameLength = 0;
	float Duration = 0;
    int NumOfJointFrames = 0;
	std::map<int, JointAnimation> JointsFrameMap;

	virtual void WriteBinary(std::ostream& out)
	{
        out.write((char*)&nameLength, sizeof(int));
		out.write(Name.c_str(), Name.size() + 1);
		out.write((char*)&Duration, sizeof(float));
        out.write((char*)&NumOfJointFrames, sizeof(int));
        //Här under loopas alla key frames igenom
		for (auto aJointAnimation : JointsFrameMap) {
			out.write((char*)&aJointAnimation.first, sizeof(int));
            out.write((char*)&aJointAnimation.second.numberOFKeyFrames, sizeof(int));
			for (auto aJointKeyFrame : aJointAnimation.second.m_KeyFrames) {
                out.write((char*)&aJointKeyFrame.Index, sizeof(int));
                out.write((char*)&aJointKeyFrame.Time, sizeof(float));
				out.write((char*)&aJointKeyFrame.Position, sizeof(float) * 3);
				out.write((char*)&aJointKeyFrame.Rotation, sizeof(float) * 4);
				out.write((char*)&aJointKeyFrame.Scale, sizeof(float) * 3);
			}
		}
	}

	virtual void WriteASCII(std::ostream& out) const
	{
		out << "Animation Name: " << Name << endl;
		out << "Duration: " << Duration << endl;
        out << "Number of KeyFrames: " << NumOfJointFrames << endl;
		for (auto aJointAnimation : JointsFrameMap) {
			out << "Bone: " << aJointAnimation.first << endl;
			//out << "Time: " << aJointAnimation.Time << endl;
            //out << "Number of Joints: " << aKeyframe.NumberOfJoints << endl;
			for (auto aJointKeyFrame :  aJointAnimation.second.m_KeyFrames) {
				//out << "Joint ID: " << aJoint.ID << endl;
                out << "Time: " << aJointKeyFrame.Time << endl;
				out << aJointKeyFrame.Position[0] << " " << aJointKeyFrame.Position[1] << " " << aJointKeyFrame.Position[2] << endl;
				out << aJointKeyFrame.Rotation[0] << " " << aJointKeyFrame.Rotation[1] << " " << aJointKeyFrame.Rotation[2] << " " << aJointKeyFrame.Rotation[3] << endl;
				out << aJointKeyFrame.Scale[0] << " " << aJointKeyFrame.Scale[1] << " " << aJointKeyFrame.Scale[2] << endl;
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