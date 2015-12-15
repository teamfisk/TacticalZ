#include "Skeleton.h"

std::vector<SkeletonNode>* Skeleton::DoIt()
{
	MItDependencyNodes jointIt(MFn::kJoint);
	SkeletonNode SkeletonStorage;

	while (!jointIt.isDone()) {
		MFnDependencyNode SkeletonFnDN(jointIt.thisNode());
		MFnTransform TransformNode(SkeletonFnDN.object());
		Joint NewJoint;

		if (MFnDependencyNode(TransformNode.parent(0)).name() == "world") {
			if (SkeletonStorage.Joints.size() != 0) {
				m_AllSkeletons.push_back(SkeletonStorage);

				SkeletonStorage.Joints.clear();
				SkeletonStorage.Name.clear();
			}

			SkeletonStorage.Name = TransformNode.name().asChar();

			NewJoint.ParentIndex = -1; // This joint is root
		}
		else {
			auto it = std::find(m_Hierarchy.begin(), m_Hierarchy.end(), TransformNode.parent(0));
			if (it != m_Hierarchy.end()) {
				NewJoint.ParentIndex = it - m_Hierarchy.begin();
			}
			else {
				MGlobal::displayError(MString() + "Could not find joint parent for: " + TransformNode.name());
			}
		}

		m_Hierarchy.push_back(TransformNode.object());

		NewJoint.Name = TransformNode.name().asChar();

		MMatrix Matrix = TransformNode.transformationMatrix();

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				NewJoint.OffsetMatrix[i][j] = Matrix.matrix[i][j];
			}
		}

		SkeletonStorage.Joints.push_back(NewJoint);

		jointIt.next();
	}

	m_AllSkeletons.push_back(SkeletonStorage);

	return &m_AllSkeletons;
}