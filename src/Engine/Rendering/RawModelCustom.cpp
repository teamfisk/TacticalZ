#include "Rendering/RawModelCustom.h"

#ifndef USING_ASSIMP_AS_IMPORTER

RawModelCustom::RawModelCustom(std::string fileName)
{
    fileName = fileName.erase(fileName.find_last_of("."), fileName.find_last_of(".") - fileName.size());
    ReadMeshFile(fileName);
    ReadMaterialFile(fileName);
    ReadAnimationFile(fileName);
    int k = 0;
}

void RawModelCustom::ReadMeshFile(std::string filePath)
{ 
    char* fileData;
    filePath += ".mesh";
    std::ifstream in(filePath.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
        throw Resource::FailedLoadingException("Open mesh file failed");
    }
    unsigned int fileByteSize = in.tellg();
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    unsigned int offset = 0;
    if (fileByteSize > 0) {
        ReadMeshFileHeader(offset, fileData, fileByteSize);
        ReadMesh(offset, fileData, fileByteSize);
    }
    delete fileData;
}

void RawModelCustom::ReadMeshFileHeader(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    m_Vertices.resize(*(unsigned int*)(fileData + offset));
    offset += sizeof(unsigned int);
    m_Indices.resize(*(unsigned int*)(fileData + offset));
    offset += sizeof(unsigned int);
#else
#endif
}

void RawModelCustom::ReadMesh(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
    ReadVertices(offset, fileData, fileByteSize);
    ReadIndices(offset, fileData, fileByteSize);
}

void RawModelCustom::ReadVertices(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    if (offset +  m_Vertices.size() * sizeof(Vertex) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading vertices failed");
    }

    memcpy(&m_Vertices[0], fileData + offset, m_Vertices.size() * sizeof(Vertex));
    offset += m_Vertices.size() * sizeof(Vertex);
#else
#endif
}

void RawModelCustom::ReadIndices(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    if (offset + m_Indices.size() * sizeof(unsigned int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading indices failed");
    }

    memcpy(&m_Indices[0], fileData + offset, m_Indices.size() * sizeof(unsigned int));
    offset += m_Indices.size() * sizeof(unsigned int);

#else
#endif
}

void RawModelCustom::ReadMaterialFile(std::string filePath)
{ 
    char* fileData;
    filePath += ".mtrl";
    std::ifstream in(filePath.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
        throw Resource::FailedLoadingException("Open material file failed");
    }
    unsigned int fileByteSize = in.tellg();
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    unsigned int offset = 0;
    if (fileByteSize > 0) {
        ReadMaterials(offset, fileData, fileByteSize);
    }
    delete fileData;
}

void RawModelCustom::ReadMaterials(unsigned int& offset, char* fileData, unsigned int& fileByteSize)
{ 
#ifdef BOOST_LITTLE_ENDIAN
    unsigned int* numMaterials = (unsigned int*)(fileData);
    MaterialGroups.reserve(*numMaterials);
    offset += sizeof(unsigned int);

    for (int i = 0; i < *numMaterials; i++) {
        ReadMaterialSingle(offset, fileData, fileByteSize);
    }
#else
#endif
}

void RawModelCustom::ReadMaterialSingle(unsigned int &offset, char* fileData, unsigned int& fileByteSize)
{ 
    MaterialGroup newMaterial;

#ifdef BOOST_LITTLE_ENDIAN

    if (offset + sizeof(unsigned int) * 4 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Material texture names length failed");
    }

    unsigned int* nameLengths = (unsigned int*)(fileData + offset); 
    offset += sizeof(unsigned int) * 4;

    if (offset + sizeof(float) * 11 + sizeof(unsigned int) * 2 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Material specular, reflection, color and start and end index  values failed");
    }

    newMaterial.SpecularExponent = *(float*)(fileData + offset);
    offset += sizeof(float);
    newMaterial.ReflectionFactor = *(float*)(fileData + offset);
    offset += sizeof(float);

    memcpy(&newMaterial.DiffuseColor[0], fileData + offset, sizeof(float) * 3);
    offset += sizeof(float) * 3;
    memcpy(&newMaterial.SpecularColor[0], fileData + offset, sizeof(float) * 3);
    offset += sizeof(float) * 3;
    memcpy(&newMaterial.IncandescenceColor[0], fileData + offset, sizeof(float) * 3);
    offset += sizeof(float) * 3;

    newMaterial.StartIndex = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);
    newMaterial.EndIndex = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    if (nameLengths[0] > 0) {
        if (offset + nameLengths[0] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material texture path failed");
        }

        newMaterial.TexturePath = "Textures/";
        newMaterial.TexturePath += (fileData + offset);
        newMaterial.TexturePath += ".png";
        offset += nameLengths[0];
    }

    if (nameLengths[1] > 0) {
        if (offset + nameLengths[1] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material NormalMap path failed");
        }
        newMaterial.NormalMapPath = "Textures/";
        newMaterial.NormalMapPath += (fileData + offset);
        newMaterial.NormalMapPath += ".png";
        offset += nameLengths[1];
    }

    if (nameLengths[2] > 0) {
        if (offset + nameLengths[2] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material SpecularMap path failed");
        }
        newMaterial.SpecularMapPath = "Textures/";
        newMaterial.SpecularMapPath += (fileData + offset);
        newMaterial.SpecularMapPath += ".png";
        offset += nameLengths[2];
    }

    if (nameLengths[3] > 0) {
        if (offset + nameLengths[3] > fileByteSize) {
            throw Resource::FailedLoadingException("Reading Material IncandescenceMap path failed");
        }
        newMaterial.IncandescenceMapPath = "Textures/";
        newMaterial.IncandescenceMapPath += (fileData + offset);
        newMaterial.IncandescenceMapPath += ".png";
        offset += nameLengths[3];
    }

#else
#endif

    MaterialGroups.push_back(newMaterial);
}

void RawModelCustom::ReadAnimationFile(std::string filePath)
{ 
    char* fileData;
    filePath += ".anim";
    std::ifstream in(filePath.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
        //throw Resource::FailedLoadingException("Open animation file failed");
        return;
    }

    unsigned int fileByteSize = in.tellg();
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    unsigned int offset = 0;
    if (fileByteSize > 0) {
        m_Skeleton = new Skeleton();

#ifdef BOOST_LITTLE_ENDIAN
        unsigned int numBindPoses = *(unsigned int*)(fileData);
        offset += sizeof(unsigned int);
        unsigned int numAnimations = *(unsigned int*)(fileData + offset);
        offset += sizeof(unsigned int);
#else
#endif

        ReadAnimationBindPoses(offset, fileData, fileByteSize);
        ReadAnimationClips(offset, fileData, fileByteSize, numAnimations);
    }
    delete fileData;
}

void RawModelCustom::ReadAnimationBindPoses(unsigned int &offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    unsigned int* numBones = (unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    for (unsigned int i = 0; i < *numBones; i++) {
        ReadAnimationJoint(offset, fileData, fileByteSize);
    }
#else
#endif
}

void RawModelCustom::ReadAnimationJoint(unsigned int &offset, char* fileData, unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
    if (offset + sizeof(unsigned int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Joint name length failed");
    }
    unsigned int jointNameLength = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    if (offset + jointNameLength > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Joint name failed");
    }
    std::string jointName = (fileData + offset);
    offset += jointNameLength;

    if (offset + sizeof(float) * 4 * 4 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Joint offset matrix failed");
    }
    glm::mat4 offsetMatrix;
    memcpy(&offsetMatrix, fileData + offset, sizeof(float) * 4 * 4);
    offset += sizeof(float) * 4 * 4;
    
    if (offset + sizeof(int) > fileByteSize) { 
        throw Resource::FailedLoadingException("Reading Joint ID failed");
    }
    int jointID = *(int*)(fileData + offset);
    offset += sizeof(int);

    if (offset + sizeof(int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading Joint Parent ID failed");
    }
    int jointParentID = *(int*)(fileData + offset);
    offset += sizeof(int);

    // Adding joint to the Skeleton
    m_Skeleton->CreateBone(jointID, jointParentID, jointName, offsetMatrix);


#else
#endif
}

void RawModelCustom::ReadAnimationClips(unsigned int &offset, char* fileData, unsigned int& fileByteSize, unsigned int numberOfClips)
{
    for (unsigned int i = 0; i < numberOfClips; i++) {
        ReadAnimationClipSingle(offset, fileData, fileByteSize, i);
    }
}

void RawModelCustom::ReadAnimationClipSingle(unsigned int &offset, char* fileData, unsigned int& fileByteSize, unsigned int clipIndex)
{
#ifdef BOOST_LITTLE_ENDIAN
    Skeleton::Animation newAnimation;

    if (offset + sizeof(unsigned int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationClip name length failed");
    }
    unsigned int clipNameLength = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    if (offset + clipNameLength > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationClip name failed");
    }
    newAnimation.Name = (fileData + offset);
    offset += clipNameLength;

    if (offset + sizeof(float) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationClip duration failed");
    }

    newAnimation.Duration = *(float*)(fileData + offset);
    offset += sizeof(float);

    if (offset + sizeof(unsigned int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationClip NrOfKeyframes failed");
    }
    unsigned int nrOfKeyframes = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    if (offset + sizeof(unsigned int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationClip NrOfJoints failed");
    }
    unsigned int nrOfJoints = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    newAnimation.Keyframes.reserve(nrOfKeyframes);
    for (unsigned int i = 0; i < nrOfKeyframes; i++) { 
        ReadAnimationKeyFrame(offset, fileData, fileByteSize, nrOfJoints, newAnimation);
    }
    m_Skeleton->Animations[newAnimation.Name] = newAnimation;
#else
#endif
}

void RawModelCustom::ReadAnimationKeyFrame(unsigned int &offset, char* fileData, unsigned int& fileByteSize, unsigned int nrOfJoints, Skeleton::Animation& animation)
{
    Skeleton::Animation::Keyframe newKeyFrame;

    if (offset + sizeof(int) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationKeyFrame index failed");
    }
    newKeyFrame.Index = *(int*)(fileData + offset);
    offset += sizeof(int);

    if (offset + sizeof(float) > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationKeyFrame time failed");
    }
    newKeyFrame.Time = *(float*)(fileData + offset);
    offset += sizeof(float);

    if (offset + sizeof(Skeleton::Animation::Keyframe::BoneProperty) * nrOfJoints> fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationKeyFrame joints failed");
    }

    Skeleton::Animation::Keyframe::BoneProperty newBone;
    for (unsigned int i = 0; i < nrOfJoints; i++) {
        memcpy(&newBone, (fileData + offset), sizeof(Skeleton::Animation::Keyframe::BoneProperty));
        offset += sizeof(Skeleton::Animation::Keyframe::BoneProperty);
        newKeyFrame.BoneProperties[newBone.ID] = newBone;
    }
    animation.Keyframes.push_back(newKeyFrame);
}

RawModelCustom::~RawModelCustom()
{
    if (m_Skeleton != nullptr) {
        delete m_Skeleton;
    }
}

#endif