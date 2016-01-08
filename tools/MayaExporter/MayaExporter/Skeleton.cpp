#include "Skeleton.h"




//std::vector<SkeletonNode> Skeleton::DoIt()
//{
//	std::vector<SkeletonNode> m_AllSkeletons;
//
//	MItDag jointIt(MItDag::TraversalType::kDepthFirst, MFn::kJoint);
//	SkeletonNode SkeletonStorage;
//
//	while (!jointIt.isDone()) {
//		MFnTransform TransformNode(jointIt.currentItem());
//		Joint NewJoint;
//
//		if (MFnDependencyNode(TransformNode.parent(0)).name() == "world") {
//			if (SkeletonStorage.Joints.size() != 0) {
//				m_AllSkeletons.push_back(SkeletonStorage);
//
//				SkeletonStorage.Joints.clear();
//				SkeletonStorage.Name.clear();
//			}
//
//			SkeletonStorage.Name = TransformNode.name().asChar();
//
//			//NewJoint.ParentIndex = -1; // This joint is root
//		}
//
//		//NewJoint.Name = TransformNode.name().asChar();
//
//		MMatrix Matrix = TransformNode.transformationMatrix();
//
//		//double tmp[3];
//		//((MTransformationMatrix)Matrix).eulerRotation().asVector().get(tmp);
//		//NewJoint.Rotation[0] = tmp[0];
//		//NewJoint.Rotation[1] = tmp[1];
//		//NewJoint.Rotation[2] = tmp[2];
//		//((MTransformationMatrix)Matrix).getScale(tmp, MSpace::Space::kTransform);
//		//NewJoint.Scale[0] = tmp[0];
//		//NewJoint.Scale[1] = tmp[1];
//		//NewJoint.Scale[2] = tmp[2];
//		//((MTransformationMatrix)Matrix).getTranslation(MSpace::Space::kTransform).get(tmp);
//		//NewJoint.Translation[0] = tmp[0];
//		//NewJoint.Translation[1] = tmp[1];
//		//NewJoint.Translation[2] = tmp[2];
//
//		for (int i = 0; i < 4; i++) {
//			for (int j = 0; j < 4; j++) {
//				NewJoint.OffsetMatrix[i][j] = Matrix.matrix[i][j];
//			}
//		}
//
//		SkeletonStorage.Joints.push_back(NewJoint);
//
//		jointIt.next();
//	}
//
//	m_AllSkeletons.push_back(SkeletonStorage);
//
//	return m_AllSkeletons;
//}
std::string attr[9] = { "scaleX", "scaleY", "scaleZ", "translateX", "translateY", "translateZ", "rotateX", "rotateY", "rotateZ" };

Animation Skeleton::GetAnimData(std::string animationName, int startFrame, int endFrame)
{
	Animation returnData;
	double oneDivSixty = 1 / 60.0;
	returnData.Name = animationName;
	returnData.Duration = (endFrame - startFrame) * oneDivSixty;
	std::vector<MObject> animatedJoints;
	std::vector<MObject> m_Hierarchy;

	MItDag jointIt(MItDag::TraversalType::kDepthFirst, MFn::kJoint);
	while (!jointIt.isDone())
	{
		m_Hierarchy.push_back(jointIt.item());

		MFnDependencyNode depNode(jointIt.item());
		for (int i = 0; i < 9; i++)
		{
			MStatus tmp;
			MPlug plug = depNode.findPlug(attr[i].c_str(), &tmp);

			MPlugArray connections;
			plug.connectedTo(connections, true, false, 0);
			for (int j = 0; j != connections.length(); j++) {
				MObject connected = connections[j].node();

				if (connected.hasFn(MFn::kAnimCurve)) {

					MFnAnimCurve jointAnim(connected);

					unsigned int startKeyFrameIndex = jointAnim.findClosest(MTime(startFrame, MTime::kNTSCField), &tmp);

					if (tmp == MStatus::kFailure)
						MGlobal::displayInfo(MString() + "Fail :c");

					if (startFrame * oneDivSixty <= jointAnim.time(startKeyFrameIndex).value() && jointAnim.time(startKeyFrameIndex).value() <= endFrame * oneDivSixty) {
						animatedJoints.push_back(jointIt.item());
						i = 9;
						break;
					}

					unsigned int endKeyFrameIndex = jointAnim.findClosest(MTime(endFrame, MTime::kNTSCField));
					MGlobal::displayInfo(MString() + startKeyFrameIndex + " " + endKeyFrameIndex);

					if (startFrame * oneDivSixty <= jointAnim.time(endKeyFrameIndex).value() && jointAnim.time(endKeyFrameIndex).value() <= endFrame * oneDivSixty || endKeyFrameIndex - startKeyFrameIndex > 0) {
						animatedJoints.push_back(jointIt.item());
						i = 9;
						break;
					}

					MFnTransform MayaJoint(jointIt.item());

					MPlug BindPose = MayaJoint.findPlug("bindPose");
					MDataHandle DataHandle;
					BindPose.getValue(DataHandle);
					MFnMatrixData MartixFn(DataHandle.data());
					MMatrix BindPoseMatrix = MartixFn.matrix();

					if (BindPoseMatrix != MayaJoint.transformationMatrix())
					{
						MGlobal::displayError(MString() + animationName.c_str() + " is using a joint that is not in bind pose nor is it key framed in the animation, the exported animation will NOT correspond to the animation in Maya");
					}
				}
			}
		}

		jointIt.next();
	}

	int currentFrame = startFrame;
	while (currentFrame != endFrame) {
		Animation::Keyframe thisKeyFrame;
		thisKeyFrame.Index = currentFrame - startFrame;
		thisKeyFrame.Time = thisKeyFrame.Index * oneDivSixty;

		MAnimControl::setCurrentTime(MTime(currentFrame, MTime::kNTSCField));
		MTime time = MAnimControl::currentTime();

		for (auto aJoint : animatedJoints){		
			MFnTransform thisJoint(aJoint);
			Animation::Keyframe::JointProperty joint;

			auto it = std::find(m_Hierarchy.begin(), m_Hierarchy.end(), thisJoint.object());
			if (it != m_Hierarchy.end()) {
				joint.ID = it - m_Hierarchy.begin();
			}
			else {
				MGlobal::displayError(MString() + "Could not find joint ID for: " + thisJoint.name());
			}
			
			MMatrix Matrix = thisJoint.transformationMatrix();
			
			double tmp[4];
			thisJoint.getRotationQuaternion(tmp[0], tmp[1], tmp[2], tmp[3]);
			joint.Rotation[0] = tmp[0];
			joint.Rotation[1] = tmp[1];
			joint.Rotation[2] = tmp[2];
			joint.Rotation[3] = tmp[3];
			thisJoint.getTranslation(MSpace::kPreTransform).get(tmp);
			joint.Position[0] = tmp[0];
			joint.Position[1] = tmp[1];
			joint.Position[2] = tmp[2];
			thisJoint.getScale(tmp);
			joint.Scale[0] = tmp[0];
			joint.Scale[1] = tmp[1];
			joint.Scale[2] = tmp[2];

			thisKeyFrame.JointProperties.push_back(joint);
		}
		returnData.Keyframes.push_back(thisKeyFrame);
		currentFrame++;
	}
	return returnData;
}

std::vector<BindPoseSkeletonNode> Skeleton::GetBindPoses()
{
	std::vector<BindPoseSkeletonNode> m_AllSkeletons;
	std::vector<MObject> m_Hierarchy;

	MItDag jointIt(MItDag::TraversalType::kDepthFirst, MFn::kJoint);
	BindPoseSkeletonNode SkeletonStorage;

	while (!jointIt.isDone()) {
		MFnTransform MayaJoint(jointIt.currentItem());
		BindPoseSkeletonNode::BindPoseJoint NewJoint;

		if (MFnDependencyNode(MayaJoint.parent(0)).name() == "world") {
			if (SkeletonStorage.Joints.size() != 0) {
				m_AllSkeletons.push_back(SkeletonStorage);

				SkeletonStorage.Joints.clear();
				SkeletonStorage.Name.clear();
			}
			SkeletonStorage.Name = std::string(MayaJoint.name().asChar());
			NewJoint.ParentID = -1; // This joint is root
		}
		else {
			auto it = std::find(m_Hierarchy.begin(), m_Hierarchy.end(), MayaJoint.parent(0));
			if (it != m_Hierarchy.end()) {
				NewJoint.ParentID = it - m_Hierarchy.begin();
			}
			else {
				MGlobal::displayError(MString() + "Could not find joint parent for: " + MayaJoint.name());
			}
		}
		m_Hierarchy.push_back(MayaJoint.object());

		MPlug BindPose = MayaJoint.findPlug("bindPose");
		MDataHandle DataHandle;
		BindPose.getValue(DataHandle);
		MFnMatrixData MartixFn(DataHandle.data());
		MMatrix Matrix = MartixFn.matrix();

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				NewJoint.OffsetMatrix[i][j] = Matrix.matrix[i][j];
			}
		}

		NewJoint.Name = MayaJoint.name().asChar();

		//double tmp[3];
		//((MTransformationMatrix)Matrix).eulerRotation().asVector().get(tmp);
		//NewJoint.Rotation[0] = tmp[0];
		//NewJoint.Rotation[1] = tmp[1];
		//NewJoint.Rotation[2] = tmp[2];
		//((MTransformationMatrix)Matrix).getScale(tmp, MSpace::Space::kTransform);
		//NewJoint.Scale[0] = tmp[0];
		//NewJoint.Scale[1] = tmp[1];
		//NewJoint.Scale[2] = tmp[2];
		//((MTransformationMatrix)Matrix).getTranslation(MSpace::Space::kTransform).get(tmp);
		//NewJoint.Translation[0] = tmp[0];
		//NewJoint.Translation[1] = tmp[1];
		//NewJoint.Translation[2] = tmp[2];
		SkeletonStorage.Joints.push_back(NewJoint);
		jointIt.next();
	}

	m_AllSkeletons.push_back(SkeletonStorage);

	return m_AllSkeletons;
}