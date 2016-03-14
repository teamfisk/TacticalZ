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

    bool Meshes(std::string pathName, bool selectedOnly = false, bool isCollision = false);
    bool Materials(std::string pathName);
    bool Animations(std::string pathName, std::vector<AnimationInfo> animInfo);

private:
    bool GetMeshData(MObjectArray object, bool collision);
    bool GetMaterialData();
    bool GetAnimationData(AnimationInfo info);

    void WriteMeshData(std::string pathName);
    void WriteAnimData(std::string pathName);
    void WriteMaterialData(std::string pathName);
	void WriteCollisionData(std::string pathName);

    Material m_MaterialHandler;
    Skeleton m_SkeletonHandler;
    MeshClass m_MeshHandler;
    

    //File export
    WriteToFile m_MeshFile;
    WriteToFile m_AnimFile;
    WriteToFile m_MtrlFile;
	WriteToFile m_ColliFile;

    //Mesh Data
    Mesh meshes;

    //Animation Data
    std::vector<BindPoseSkeletonNode> allBindPoses;
    std::vector<Animation> allAnimations;

    //Material Data
    std::vector<MaterialNode>* AllMaterials;
};
#endif