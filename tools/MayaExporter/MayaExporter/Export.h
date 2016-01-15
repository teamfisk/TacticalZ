#ifndef Export_Export_h__
#define Export_Export_h__

#include <vector>
#include <string>

#include "MayaIncludes.h"
#include "Material.h"
#include "Mesh.h"
#include "Skeleton.h"
#include "WriteToFile.h"

class Export {
public:
    Export();

    ~Export();

    struct AnimationInfo {
        std::string Name;
        int Start;
        int End;
    };

    bool Meshes(std::string pathName, bool selectedOnly = false);
    bool Materials();
    bool Animations(std::string pathName, std::vector<AnimationInfo> animInfo);

private:
    bool GetMeshData(MObject object);
    bool GetMaterialData();
    bool GetAnimationData(AnimationInfo info);

    void WriteMeshData(std::string pathName);
    void WriteAnimData(std::string pathName);

    Material m_MaterialHandler;
    Skeleton m_SkeletonHandler;
    MeshClass m_MeshHandler;
    

    //File export
    WriteToFile m_MeshFile;
    WriteToFile m_AnimFile;

    //Mesh Data
    std::vector<Mesh> meshes;

    //Animation Data
    std::vector<BindPoseSkeletonNode> allBindPoses;
    std::vector<Animation> allAnimations;

};
#endif