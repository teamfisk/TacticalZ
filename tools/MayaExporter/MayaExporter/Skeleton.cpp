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
static std::string attr[9] = { "scaleX", "scaleY", "scaleZ", "translateX", "translateY", "translateZ", "rotateX", "rotateY", "rotateZ" };

Animation Skeleton::GetAnimData(std::string animationName, int startFrame, int endFrame)
{
    MStatus status;
    std::vector<MObject> animatedJoints;
    std::vector<MObject> m_Hierarchy;

	Animation returnData;
	double oneDivSixty = 1 / 60.0;
	returnData.Name = animationName;
    returnData.nameLength = animationName.size() + 1;
	returnData.Duration = (endFrame - startFrame) * oneDivSixty;

	std::map<std::string, std::array<std::array<double, 4>, 4>> joinCheckMap;
	std::map<std::string, bool> exportJoint;

	MItDag jointIt(MItDag::TraversalType::kDepthFirst, MFn::kJoint);
	for (unsigned int i = startFrame; i <= endFrame; i++)
	{
		MAnimControl::setCurrentTime(MTime(i, MTime::kNTSCField));
		while (!jointIt.isDone())
		{
			m_Hierarchy.push_back(jointIt.item());

			MFnTransform MayaJoint(jointIt.item());
			MMatrix transformationMatrix = MayaJoint.transformationMatrix();

			if (i == startFrame)
			{
				double doubleMat[4][4];
				transformationMatrix.get(doubleMat);

				joinCheckMap[MayaJoint.name().asChar()][0][0] = doubleMat[0][0];
				joinCheckMap[MayaJoint.name().asChar()][0][1] = doubleMat[0][1];
				joinCheckMap[MayaJoint.name().asChar()][0][2] = doubleMat[0][2];
				joinCheckMap[MayaJoint.name().asChar()][0][3] = doubleMat[0][3];
				joinCheckMap[MayaJoint.name().asChar()][1][0] = doubleMat[1][0];
				joinCheckMap[MayaJoint.name().asChar()][1][1] = doubleMat[1][1];
				joinCheckMap[MayaJoint.name().asChar()][1][2] = doubleMat[1][2];
				joinCheckMap[MayaJoint.name().asChar()][1][3] = doubleMat[1][3];
				joinCheckMap[MayaJoint.name().asChar()][2][0] = doubleMat[2][0];
				joinCheckMap[MayaJoint.name().asChar()][2][1] = doubleMat[2][1];
				joinCheckMap[MayaJoint.name().asChar()][2][2] = doubleMat[2][2];
				joinCheckMap[MayaJoint.name().asChar()][2][3] = doubleMat[2][3];
				joinCheckMap[MayaJoint.name().asChar()][3][0] = doubleMat[3][0];
				joinCheckMap[MayaJoint.name().asChar()][3][1] = doubleMat[3][1];
				joinCheckMap[MayaJoint.name().asChar()][3][2] = doubleMat[3][2];
				joinCheckMap[MayaJoint.name().asChar()][3][3] = doubleMat[3][3];

				exportJoint[MayaJoint.name().asChar()] = false;
			}
			else if(!exportJoint[MayaJoint.name().asChar()])//!exportJoint[MayaJoint.name().asChar()])
			{
				double doubleMat[4][4];

				doubleMat[0][0] = joinCheckMap[MayaJoint.name().asChar()][0][0];
				doubleMat[0][1] = joinCheckMap[MayaJoint.name().asChar()][0][1];
				doubleMat[0][2] = joinCheckMap[MayaJoint.name().asChar()][0][2];
				doubleMat[0][3] = joinCheckMap[MayaJoint.name().asChar()][0][3];
				doubleMat[1][0] = joinCheckMap[MayaJoint.name().asChar()][1][0];
				doubleMat[1][1] = joinCheckMap[MayaJoint.name().asChar()][1][1];
				doubleMat[1][2] = joinCheckMap[MayaJoint.name().asChar()][1][2];
				doubleMat[1][3] = joinCheckMap[MayaJoint.name().asChar()][1][3];
				doubleMat[2][0] = joinCheckMap[MayaJoint.name().asChar()][2][0];
				doubleMat[2][1] = joinCheckMap[MayaJoint.name().asChar()][2][1];
				doubleMat[2][2] = joinCheckMap[MayaJoint.name().asChar()][2][2];
				doubleMat[2][3] = joinCheckMap[MayaJoint.name().asChar()][2][3];
				doubleMat[3][0] = joinCheckMap[MayaJoint.name().asChar()][3][0];
				doubleMat[3][1] = joinCheckMap[MayaJoint.name().asChar()][3][1];
				doubleMat[3][2] = joinCheckMap[MayaJoint.name().asChar()][3][2];
				doubleMat[3][3] = joinCheckMap[MayaJoint.name().asChar()][3][3];

				MMatrix tmp(doubleMat);
				if (!tmp.isEquivalent(transformationMatrix)) {
					MGlobal::displayInfo(MString() + MayaJoint.name() + " is exported");
					exportJoint[MayaJoint.name().asChar()] = true;
					animatedJoints.push_back(MayaJoint.object());
				}
			}

				/*for (int i = 0; i < 9; i++)
				{
					MStatus tmp;
					MPlug plug = depNode.findPlug(attr[i].c_str(), &tmp);

					MPlugArray connections;
					plug.connectedTo(connections, true, false, 0);
					for (int j = 0; j != connections.length(); j++) {
						MObject connected = connections[j].node();

						if (connected.hasFn(MFn::kAnimCurve)) {

							MFnAnimCurve jointAnim(connected);

                    //MGlobal::displayInfo(MString() + "curve                     : " + jointAnim.name());
                    //MGlobal::displayInfo(MString() + "curve keys                : " + jointAnim.numKeys());
                    //MGlobal::displayInfo(MString() + "curve keyframes           : " + jointAnim.numKeyframes());
                    //MGlobal::displayInfo(MString() + "startFrame                : " + startFrame);
                    //MGlobal::displayInfo(MString() + "endFrame                  : " + endFrame);

							unsigned int startKeyFrameIndex = jointAnim.findClosest(MTime(startFrame, MTime::kNTSCField), &tmp);

							if (tmp == MStatus::kFailure)
								MGlobal::displayInfo(MString() + "Fail :c");

							if (startFrame * oneDivSixty <= jointAnim.time(startKeyFrameIndex).value() && jointAnim.time(startKeyFrameIndex).value() <= endFrame * oneDivSixty) {
                    //MGlobal::displayInfo(MString() + "Start key time            : " + jointAnim.time(startKeyFrameIndex).value());


					if (startFrame <= jointAnim.time(startKeyFrameIndex).value() && jointAnim.time(startKeyFrameIndex).value() <= endFrame ) {
								animatedJoints.push_back(jointIt.item());
								i = 9;
								break;
							}

							unsigned int endKeyFrameIndex = jointAnim.findClosest(MTime(endFrame, MTime::kNTSCField));
							MGlobal::displayInfo(MString() + startKeyFrameIndex + " " + endKeyFrameIndex);
                        MGlobal::displayInfo(MString() + "Fail!!!!!!!!!!!!!!!!!!!!!!!!!");

							if (startFrame * oneDivSixty <= jointAnim.time(endKeyFrameIndex).value() && jointAnim.time(endKeyFrameIndex).value() <= endFrame * oneDivSixty || endKeyFrameIndex - startKeyFrameIndex > 0) {
                    //MGlobal::displayInfo(MString() + "End key index             : " + endKeyFrameIndex);
                    //MGlobal::displayInfo(MString() + "end key time              : " + jointAnim.time(endKeyFrameIndex).value());

					/*MGlobal::displayInfo(MString() + "start keyfram index: " + startKeyFrameIndex + ". End keyfram index: " + endKeyFrameIndex + ".");*/

					if (startFrame  <= jointAnim.time(endKeyFrameIndex).value() && jointAnim.time(endKeyFrameIndex).value() <= endFrame || endKeyFrameIndex - startKeyFrameIndex > 0) {
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

							if (!BindPoseMatrix.isEquivalent(MayaJoint.transformationMatrix()))
							{
								MGlobal::displayError(MString() + animationName.c_str() + " is using " + MayaJoint.name() + " that is not in bind pose nor is it key framed in the animation, the exported animation will NOT correspond to the animation in Maya");
							}
						}
					}
				}*/

		} // end of int i loop
				jointIt.next();
		}
		jointIt.reset();
	}


	int currentFrame = startFrame;
	while (currentFrame <= endFrame) { // ANDREAS
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
			
			MTransformationMatrix Matrix = thisJoint.transformation();
            MPlug BindPose = thisJoint.findPlug("bindPose");
            MDataHandle DataHandle;
            BindPose.getValue(DataHandle);
            MFnMatrixData MartixFn(DataHandle.data());
            MMatrix BindPoseMatrix = MartixFn.matrix();
            Matrix = Matrix.asMatrix();
            
            MObject jointOrientObj = thisJoint.attribute("jointOrient");
            MFnNumericAttribute jointOrient(jointOrientObj);
            double jointOrientDouble[3];
            jointOrient.getDefault(jointOrientDouble[0], jointOrientDouble[1], jointOrientDouble[2]);
            //MGlobal::displayError(MString() + "Joint Matrix: ");
            //MGlobal::displayError(MString() + Matrix.asMatrix()[0][0] + " " + Matrix.asMatrix()[0][1] + " " + Matrix.asMatrix()[0][2] + " " + Matrix.asMatrix()[0][3]);
            //MGlobal::displayError(MString() + Matrix.asMatrix()[1][0] + " " + Matrix.asMatrix()[1][1] + " " + Matrix.asMatrix()[1][2] + " " + Matrix.asMatrix()[1][3]);
            //MGlobal::displayError(MString() + Matrix.asMatrix()[2][0] + " " + Matrix.asMatrix()[2][1] + " " + Matrix.asMatrix()[2][2] + " " + Matrix.asMatrix()[2][3]);
            //MGlobal::displayError(MString() + Matrix.asMatrix()[3][0] + " " + Matrix.asMatrix()[3][1] + " " + Matrix.asMatrix()[3][2] + " " + Matrix.asMatrix()[3][3]);

            MEulerRotation joEuler(jointOrientDouble[0], jointOrientDouble[1], jointOrientDouble[2]);
            MQuaternion jo = joEuler.asQuaternion();

			double tmp[4];
            Matrix.getRotationQuaternion(tmp[0], tmp[1], tmp[2], tmp[3]);
            MQuaternion rotation(tmp);

            rotation = rotation * jo;
            rotation.get(tmp);

			joint.Rotation[0] = tmp[0];
			joint.Rotation[1] = tmp[1];
			joint.Rotation[2] = tmp[2];
			joint.Rotation[3] = tmp[3];
            Matrix.getTranslation(MSpace::kTransform).get(tmp);
			joint.Position[0] = tmp[0];
			joint.Position[1] = tmp[1];
			joint.Position[2] = tmp[2];
            Matrix.getScale(tmp, MSpace::kTransform);
			joint.Scale[0] = tmp[0];
			joint.Scale[1] = tmp[1];
			joint.Scale[2] = tmp[2];

			thisKeyFrame.JointProperties.push_back(joint);
		}
		returnData.Keyframes.push_back(thisKeyFrame);
		currentFrame++;
	}

    returnData.NumKeyFrames = returnData.Keyframes.size();
    returnData.NumberOfJoints = animatedJoints.size();

	return returnData;
}

std::vector<BindPoseSkeletonNode> Skeleton::GetBindPoses()
{
    MStatus status;
	std::vector<BindPoseSkeletonNode> m_AllSkeletons;
	std::vector<MObject> m_Hierarchy;

	MItDag jointIt(MItDag::TraversalType::kDepthFirst, MFn::kJoint);
	BindPoseSkeletonNode SkeletonStorage;
	while (!jointIt.isDone()) {
		MFnTransform MayaJoint(jointIt.currentItem());
		BindPoseSkeletonNode::BindPoseJoint NewJoint;

		if (MFnDependencyNode(MayaJoint.parent(0)).object().apiType() != MFn::kJoint) {
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

		MPlug BindPose = MayaJoint.findPlug("bindPose", &status);
        if (status != MS::kSuccess) { 
            MGlobal::displayError(MString() + "Could not find bindPose plug: " + status.errorString());
        }
		MDataHandle DataHandle;
		BindPose.getValue(DataHandle);
		MFnMatrixData MartixFn(DataHandle.data());
		MMatrix Matrix = MartixFn.matrix();


        MVector tmp = MayaJoint.transformation().getTranslation(MSpace::kObject);
        //Matrix[3][0] *= -1;
        //Matrix[3][2] *= -1;
        //Matrix[3][1] *= -1;

        double test[3];
        MayaJoint.transformation().getScale(test, MSpace::kObject);

        MTransformationMatrix::RotationOrder order = MTransformationMatrix::RotationOrder::kXYZ;
        MayaJoint.transformation().getRotation(test, order);
       
        //----- test


        //MDataHandle DataHandle;
        //MObject jointObject(jointIt.currentItem());
        //MFnDependencyNode jointDependNode(jointObject);
        //MPlug worldMatrixArray(jointObject, jointDependNode.attribute("worldMatrix"));

        //MMatrix Matrix;
        //for (int i = 0; i < worldMatrixArray.numElements(); i++) {
        //    MPlugArray connections;

        //    MPlug element = worldMatrixArray[i];
        //    unsigned int logicalIndex = element.logicalIndex();

        //    MItDependencyGraph it(element, MFn::kSkinClusterFilter);

        //    for (; !it.isDone(); it.next()) {
        //        MFnSkinCluster skinCluster(it.thisNode());

        //        MPlug bindPreMatrixArrayPlug =
        //            skinCluster.findPlug("bindPreMatrix", &status);

        //        if (status != MS::kSuccess) {
        //            MGlobal::displayError(MString() + "Could not find bindPreMatrix plug: " + status.errorString());
        //            break;
        //        }

        //        MPlug bindPreMatrixPlug =
        //            bindPreMatrixArrayPlug.elementByLogicalIndex(logicalIndex);
        //        MObject dataObject;
        //        bindPreMatrixPlug.getValue(dataObject);

        //        MFnMatrixData matDataFn(dataObject);

        //        MMatrix invMat = matDataFn.matrix();
        //        Matrix = invMat.inverse();
        //    }
        //}


        //----- end test


        Matrix = Matrix.inverse();

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				NewJoint.OffsetMatrix[i][j] = Matrix.matrix[i][j];
			}
		}

		NewJoint.Name = MayaJoint.name().asChar();
        NewJoint.NameLength = MayaJoint.name().length() + 1;
        NewJoint.ID = SkeletonStorage.Joints.size();
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
        SkeletonStorage.numBones++;
		jointIt.next();
	}
	m_AllSkeletons.push_back(SkeletonStorage);

	return m_AllSkeletons;
}