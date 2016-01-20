#include "Export.h"

Export::Export()
{ 

}

bool Export::Meshes(std::string pathName, bool selectedOnly)
{ 
    if (pathName.empty()) {
        MGlobal::displayError(MString() + "Export::Meshes() got no pathName. Do not know where to write file");
        return false;
    }
    MSelectionList selectedOnStart;
    MGlobal::getActiveSelectionList(selectedOnStart);
   
    MObjectArray Objects;
    if (selectedOnly) {
        // Retrieving the objects we currently have selected
        MSelectionList selected;
        MGlobal::getActiveSelectionList(selected);

        // Loop through or list of selection(s)
        for (unsigned int i = 0; i < selected.length(); i++) {
            MObject object;
            selected.getDependNode(i, object);

            if (object.hasFn(MFn::kMesh)) {
                MFnDependencyNode thisNode(object);
                Objects.append(object);
            }
        }
    } else {
        // Loop through all nodes in the scene
        MItDependencyNodes it(MFn::kMesh);
        for (; !it.isDone(); it.next()) {
            MObject node = it.thisNode();
            MFnDependencyNode thisNode(node);
            MPlugArray connections;
            thisNode.findPlug("inMesh").connectedTo(connections, true, true);

            for (unsigned int i = 0; i < connections.length(); i++) {
                if (connections[i].node().apiType() == MFn::kSkinClusterFilter) {
                    MGlobal::select(node);
                    MGlobal::executeCommand("gotoBindPose");
                }
            }

            Objects.append(node);
        }
    }
    GetMeshData(Objects);
    WriteMeshData(pathName);
    return true;
}

bool Export::Materials(std::string pathName)
{ 
    if (!GetMaterialData())
        return false;
    WriteMaterialData(pathName);
    return true;
}

bool Export::Animations(std::string pathName, std::vector<AnimationInfo> animInfo)
{ 
    if (MAnimControl::currentTime().unit() != MTime::kNTSCField) {

        MGlobal::displayError(MString() + "Please change to 60 FPS under Preferences/Settings!");
        return false;
    }

    if (pathName.empty()) {
        MGlobal::displayError(MString() + "Export::Animations() got no pathName. Do not know where to write file");
        return false;
    }
    allBindPoses = m_SkeletonHandler.GetBindPoses();

    for (auto clip : animInfo) {
        if (!GetAnimationData(clip)) {
            MGlobal::displayError(MString() + "Export::Animations() failed to export " + clip.Name.c_str());
            return false;
        }
    }
    WriteAnimData(pathName);
    return true;
}

bool Export::GetMeshData(MObjectArray object)
{ 
    meshes = m_MeshHandler.GetMeshData(object);
    return true;
}

bool Export::GetMaterialData()
{ 
    // Traverse scene and return vector with all materials
    AllMaterials = m_MaterialHandler.DoIt(meshes);

    return true;
}

bool Export::GetAnimationData(AnimationInfo animInfo)
{
    if (animInfo.Name.empty()) {
        MGlobal::displayError(MString() + "A clip does not have a name");
        return false;
    }
    if (animInfo.End - animInfo.Start <= 0) {
        MGlobal::displayError(MString() + "A clip ends before it starts or contains 0 frames");
        return false;
    }

    allAnimations.push_back(m_SkeletonHandler.GetAnimData(animInfo.Name, animInfo.Start, animInfo.End));
    return true;
}

void Export::WriteMeshData(std::string pathName)
{
    m_MeshFile.ASCIIFilePath(pathName +"_mesh.txt");
    m_MeshFile.binaryFilePath(pathName + ".mesh");

    m_MeshFile.OpenFiles();

    m_MeshFile.writeToFiles((OutputData*)&meshes);

    m_MeshFile.CloseFiles();
}

void Export::WriteAnimData(std::string pathName)
{
    if (allBindPoses.size() > 0) {
        m_AnimFile.ASCIIFilePath(pathName + "_anim.txt");
        m_AnimFile.binaryFilePath(pathName + ".anim");

        m_AnimFile.OpenFiles();
        int size = allBindPoses.size();
        m_AnimFile.writeToFiles(&size);

        
        size = allAnimations.size();
        m_AnimFile.writeToFiles(&size);

        //print out all bind poses
        for (auto aBindPose : allBindPoses) {
            m_AnimFile.writeToFiles((OutputData*)&aBindPose);
        }
        for (auto aAnimation : allAnimations) {
            m_AnimFile.writeToFiles((OutputData*)&aAnimation);
        }

        m_AnimFile.CloseFiles();
    } else
        MGlobal::displayInfo("Export::WriteAnimData() got called when allBindPoses contained no data, did not write nor created them");
}

void Export::WriteMaterialData(std::string pathName)
{ 
    m_MtrlFile.ASCIIFilePath(pathName +"_mtrl.txt");
    m_MtrlFile.binaryFilePath(pathName + ".mtrl");

    m_MtrlFile.OpenFiles();

    int size = (*AllMaterials).size();
    m_MtrlFile.writeToFiles(&size);

    for (auto aMaterial : *AllMaterials) {
        m_MtrlFile.writeToFiles((OutputData*)&aMaterial);
    }
    m_MtrlFile.CloseFiles();
}

Export::~Export()
{
   /* delete m_MaterialHandler;
    delete m_SkeletonHandler;
    delete m_MeshHandler;*/

}