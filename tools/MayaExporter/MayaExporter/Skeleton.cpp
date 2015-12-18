#include "Skeleton.h"

std::vector<SkeletonNode> Skeleton::DoIt()
{
	std::vector<SkeletonNode> m_AllSkeletons;

	MItDag jointIt(MItDag::TraversalType::kDepthFirst, MFn::kJoint);
	SkeletonNode SkeletonStorage;

	while (!jointIt.isDone()) {
		MFnTransform TransformNode(jointIt.currentItem());
		Joint NewJoint;

		if (MFnDependencyNode(TransformNode.parent(0)).name() == "world") {
			if (SkeletonStorage.Joints.size() != 0) {
				m_AllSkeletons.push_back(SkeletonStorage);

				SkeletonStorage.Joints.clear();
				SkeletonStorage.Name.clear();
			}

			SkeletonStorage.Name = TransformNode.name().asChar();

			//NewJoint.ParentIndex = -1; // This joint is root
		}

		//NewJoint.Name = TransformNode.name().asChar();

		MMatrix Matrix = TransformNode.transformationMatrix();

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

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				NewJoint.OffsetMatrix[i][j] = Matrix.matrix[i][j];
			}
		}

		SkeletonStorage.Joints.push_back(NewJoint);

		jointIt.next();
	}

	m_AllSkeletons.push_back(SkeletonStorage);

	return m_AllSkeletons;
}

std::vector<BindPoseSkeletonNode> Skeleton::GetBindPoses()
{
	std::vector<BindPoseSkeletonNode> m_AllSkeletons;
	std::vector<MObject> m_Hierarchy;

	MItDag jointIt(MItDag::TraversalType::kDepthFirst, MFn::kJoint);
	BindPoseSkeletonNode SkeletonStorage;

	while (!jointIt.isDone()) {
		MFnTransform MayaJoint(jointIt.currentItem());

		if (MFnDependencyNode(MayaJoint.parent(0)).name() == "world") {
			if (SkeletonStorage.Joints.size() != 0) {
				m_AllSkeletons.push_back(SkeletonStorage);

				SkeletonStorage.Joints.clear();
				SkeletonStorage.Name.clear();
				SkeletonStorage.ParentIDs.clear();
				SkeletonStorage.Name.clear();
			}

			SkeletonStorage.Name = MayaJoint.name().asChar();
			SkeletonStorage.ParentIDs.push_back(-1); // This joint is root
		}
		else {
			auto it = std::find(m_Hierarchy.begin(), m_Hierarchy.end(), MayaJoint.parent(0));
			if (it != m_Hierarchy.end()) {
				SkeletonStorage.ParentIDs.push_back(it - m_Hierarchy.begin());
			}
			else {
				MGlobal::displayError(MString() + "Could not find joint parent for: " + MayaJoint.name());
			}
		}
		m_Hierarchy.push_back(MayaJoint.object());

		Joint NewJoint;

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

		/*double tmp[3];
		((MTransformationMatrix)Matrix).eulerRotation().asVector().get(tmp);
		NewJoint.Rotation[0] = tmp[0];
		NewJoint.Rotation[1] = tmp[1];
		NewJoint.Rotation[2] = tmp[2];
		((MTransformationMatrix)Matrix).getScale(tmp, MSpace::Space::kTransform);
		NewJoint.Scale[0] = tmp[0];
		NewJoint.Scale[1] = tmp[1];
		NewJoint.Scale[2] = tmp[2];
		((MTransformationMatrix)Matrix).getTranslation(MSpace::Space::kTransform).get(tmp);
		NewJoint.Translation[0] = tmp[0];
		NewJoint.Translation[1] = tmp[1];
		NewJoint.Translation[2] = tmp[2];*/

		SkeletonStorage.Joints.push_back(NewJoint);
		SkeletonStorage.JointNames.push_back(MayaJoint.name().asChar());

		jointIt.next();
	}

	m_AllSkeletons.push_back(SkeletonStorage);

	return m_AllSkeletons;
}