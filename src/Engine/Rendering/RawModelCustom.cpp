#include "Rendering/RawModelCustom.h"

#ifndef USING_ASSIMP_AS_IMPORTER

RawModelCustom::RawModelCustom(std::string fileName)
{
    if(fileName.substr(fileName.find_last_of(".")).compare(".mesh") != 0) {
        throw Resource::FailedLoadingException("Unknown model file format. Please use \".mesh\" files.");
    }
    fileName = fileName.erase(fileName.find_last_of("."), fileName.find_last_of(".") - fileName.size());
    ReadMeshFile(fileName);
    ReadMaterialFile(fileName);
    ReadAnimationFile(fileName);
}

void RawModelCustom::ReadMeshFile(std::string filePath)
{ 
    char* fileData;
    filePath += ".mesh";
    std::ifstream in(filePath.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
        throw Resource::FailedLoadingException("Open mesh file failed");
    }
    unsigned int fileByteSize = static_cast<unsigned int>(in.tellg());
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    std::size_t offset = 0;
    if (fileByteSize > 0) {
        ReadMeshFileHeader(offset, fileData);
        ReadMesh(offset, fileData, fileByteSize);
    }
    delete[] fileData;
}

void RawModelCustom::ReadMeshFileHeader(std::size_t& offset, char* fileData)
{
#ifdef BOOST_LITTLE_ENDIAN
	hasSkin = *(bool*)(fileData + offset);
	offset += sizeof(bool);
	if (hasSkin) {
		m_SkinedVertices.resize(static_cast<std::size_t>(*(unsigned int*)(fileData + offset)));
	}
	else {
		m_Vertices.resize(static_cast<std::size_t>(*(unsigned int*)(fileData + offset)));
	}
    offset += sizeof(unsigned int);
    m_Indices.resize(static_cast<std::size_t>(*(unsigned int*)(fileData + offset)));
    offset += sizeof(unsigned int);
#else
#endif
}

void RawModelCustom::ReadMesh(std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
{
    ReadVertices(offset, fileData, fileByteSize);
    ReadIndices(offset, fileData, fileByteSize);
}

void RawModelCustom::ReadVertices(std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
{
#ifdef BOOST_LITTLE_ENDIAN
	if (hasSkin) {
		if (offset + m_SkinedVertices.size() * sizeof(SkinedVertex) > fileByteSize) {
			throw Resource::FailedLoadingException("Reading skined vertices failed");
		}
		memcpy(&m_SkinedVertices[0], fileData + offset, m_SkinedVertices.size() * sizeof(SkinedVertex));
		offset += m_SkinedVertices.size() * sizeof(SkinedVertex);
	} else {
		if (offset + m_Vertices.size() * sizeof(Vertex) > fileByteSize) {
			throw Resource::FailedLoadingException("Reading vertices failed");
		}
		memcpy(&m_Vertices[0], fileData + offset, m_Vertices.size() * sizeof(Vertex));
		offset += m_Vertices.size() * sizeof(Vertex);
	}
#else
#endif
}

void RawModelCustom::ReadIndices(std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
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

    unsigned int fileByteSize = static_cast<unsigned int>(in.tellg());
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    std::size_t offset = 0;
    if (fileByteSize > 0) {
        ReadMaterials(offset, fileData, fileByteSize);
    }
    delete[] fileData;
}

void RawModelCustom::ReadMaterials(std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
{ 
#ifdef BOOST_LITTLE_ENDIAN
    unsigned int* numMaterials = (unsigned int*)(fileData);
	m_Materials.reserve(*numMaterials);
    offset += sizeof(unsigned int);

    for (unsigned int i = 0; i < *numMaterials; i++) {
        ReadMaterialSingle(offset, fileData, fileByteSize);
    }
#else
#endif
}

void RawModelCustom::ReadMaterialSingle(std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
{ 
	MaterialProperties newMaterialProperty;
#ifdef BOOST_LITTLE_ENDIAN

	if (offset + sizeof(MaterialType) > fileByteSize) {
		throw Resource::FailedLoadingException("Reading Material Type failed");
	}
	MaterialType type = *(MaterialType*)(fileData + offset);
	offset += sizeof(MaterialType);

	switch (type) {
	case MaterialType::Basic:
		newMaterialProperty.material = new MaterialBasic();
		ReadMaterialBasic(newMaterialProperty.material, offset, fileData, fileByteSize);
		if(hasSkin){
			newMaterialProperty.ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSkinnedProgram")->ResourceID;
		} else {
			newMaterialProperty.ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram")->ResourceID;
		}
		break;
	case MaterialType::SplatMapping:
		newMaterialProperty.material = new MaterialSplatMapping();
		ReadMaterialSplatMapping(static_cast<MaterialSplatMapping*>(newMaterialProperty.material), offset, fileData, fileByteSize);
		if (hasSkin){
			newMaterialProperty.ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapSkinnedProgram")->ResourceID;
		} else {
			newMaterialProperty.ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSplatMapProgram")->ResourceID;
		}
		break;
	case MaterialType::SingleTextures:
		newMaterialProperty.material = new MaterialSingleTextures();
		ReadMaterialSingleTexture(static_cast<MaterialSingleTextures*>(newMaterialProperty.material), offset, fileData, fileByteSize);
		if (hasSkin){
			newMaterialProperty.ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusSkinnedProgram")->ResourceID;
		} else {
			newMaterialProperty.ShaderID = ResourceManager::Load<ShaderProgram>("#ForwardPlusProgram")->ResourceID;
		}
		break;
	default:
		throw Resource::FailedLoadingException("Material contains an unknown MaterialType");
	};

	newMaterialProperty.type = type;

#else
#endif

    m_Materials.push_back(newMaterialProperty);
}

void RawModelCustom::ReadMaterialBasic(RawModelCustom::MaterialBasic* newMaterial, std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
{
	if (offset + sizeof(float) * 11 + sizeof(unsigned int) * 2 > fileByteSize) {
		throw Resource::FailedLoadingException("Reading Material specular, reflection, color and start and end index  values failed");
	}

	newMaterial->SpecularExponent = *(float*)(fileData + offset);
	offset += sizeof(float);
	newMaterial->ReflectionFactor = *(float*)(fileData + offset);
	offset += sizeof(float);

	memcpy(&newMaterial->DiffuseColor[0], fileData + offset, sizeof(float) * 3);
	offset += sizeof(float) * 3;
	memcpy(&newMaterial->SpecularColor[0], fileData + offset, sizeof(float) * 3);
	offset += sizeof(float) * 3;
	memcpy(&newMaterial->IncandescenceColor[0], fileData + offset, sizeof(float) * 3);
	offset += sizeof(float) * 3;

	newMaterial->StartIndex = *(unsigned int*)(fileData + offset);
	offset += sizeof(unsigned int);
	newMaterial->EndIndex = *(unsigned int*)(fileData + offset);
	offset += sizeof(unsigned int);
}

void RawModelCustom::ReadMaterialSingleTexture(RawModelCustom::MaterialSingleTextures* newMaterial, std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
{	
	ReadMaterialBasic(newMaterial, offset, fileData, fileByteSize);
	if (offset + sizeof(unsigned char) * 4 > fileByteSize) {
		throw Resource::FailedLoadingException("Reading Material NumOfMaps failed");
	}
	unsigned char numberOfMaps[4];
	memcpy(numberOfMaps, fileData + offset, sizeof(unsigned char) * 4);
	offset += sizeof(unsigned char) * 4;

	if (numberOfMaps[0] > 0)
	{
		ReadMaterialTextureProperties(newMaterial->ColorMap, offset, fileData, fileByteSize);
	}

	if (numberOfMaps[1] > 0)
	{
		ReadMaterialTextureProperties(newMaterial->SpecularMap, offset, fileData, fileByteSize);
	}

	if (numberOfMaps[2] > 0)
	{
		ReadMaterialTextureProperties(newMaterial->NormalMap, offset, fileData, fileByteSize);
	}

	if (numberOfMaps[3] > 0)
	{
		ReadMaterialTextureProperties(newMaterial->IncandescenceMap, offset, fileData, fileByteSize);
	}
}

void RawModelCustom::ReadMaterialSplatMapping(RawModelCustom::MaterialSplatMapping* newMaterial, std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
{
	ReadMaterialBasic(newMaterial, offset, fileData, fileByteSize);
	ReadMaterialTextureProperties(newMaterial->SplatMap, offset, fileData, fileByteSize);
	if (offset + sizeof(unsigned char) * 4 > fileByteSize) {
		throw Resource::FailedLoadingException("Reading Material NumOfMaps failed");
	}

	unsigned char numberOfMaps[4];
	memcpy(numberOfMaps, fileData + offset, sizeof(unsigned char) * 4);
	offset += sizeof(unsigned char) * 4;

	newMaterial->ColorMaps.resize(numberOfMaps[0]);
	for (unsigned char i = 0; i < numberOfMaps[0]; i++)
	{	
		ReadMaterialTextureProperties(newMaterial->ColorMaps[i], offset, fileData, fileByteSize);
	}

	newMaterial->SpecularMaps.resize(numberOfMaps[1]);
	for (unsigned char i = 0; i < numberOfMaps[1]; i++)
	{
		ReadMaterialTextureProperties(newMaterial->SpecularMaps[i], offset, fileData, fileByteSize);
	}

	newMaterial->NormalMaps.resize(numberOfMaps[2]);
	for (unsigned char i = 0; i < numberOfMaps[2]; i++)
	{
		ReadMaterialTextureProperties(newMaterial->NormalMaps[i], offset, fileData, fileByteSize);
	}

	newMaterial->IncandescenceMaps.resize(numberOfMaps[3]);
	for (unsigned char i = 0; i < numberOfMaps[3]; i++)
	{
		ReadMaterialTextureProperties(newMaterial->IncandescenceMaps[i], offset, fileData, fileByteSize);
	}
}

void RawModelCustom::ReadMaterialTextureProperties(RawModelCustom::TextureProperties& texture, std::size_t& offset, char* fileData, const unsigned int& fileByteSize) {
	unsigned int nameLength = *(unsigned int*)(fileData + offset);
	offset += sizeof(unsigned int);

	if (nameLength > 0) {
		if (offset + nameLength > fileByteSize) {
			throw Resource::FailedLoadingException("Reading Material texture path failed");
		}
		texture.TexturePath = "Textures/";
		texture.TexturePath += (fileData + offset);
		texture.TexturePath += ".png";
		offset += nameLength;
		if (offset + sizeof(glm::vec2) > fileByteSize) {
			throw Resource::FailedLoadingException("Reading Material texture UVTiling failed");
		}
		memcpy(&texture.UVRepeat[0], fileData + offset, sizeof(glm::vec2));
	}
	offset += sizeof(glm::vec2);
}

void RawModelCustom::ReadAnimationFile(std::string filePath)
{ 
    char* fileData;
    filePath += ".anim";
    std::ifstream in(filePath.c_str(), std::ios_base::binary | std::ios_base::ate);

    if (!in.is_open()) {
        if (hasSkin) {
            throw Resource::FailedLoadingException("Open animation file for a skinned mesh failed, unknown stuff will happen");
        }
        return;
    }

    unsigned int fileByteSize = static_cast<unsigned int>(in.tellg());
    in.seekg(0, std::ios_base::beg);

    fileData = new char[fileByteSize];
    in.read(fileData, fileByteSize);
    in.close();

    std::size_t offset = 0;
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
    delete[] fileData;
}

void RawModelCustom::ReadAnimationBindPoses(std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
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

void RawModelCustom::ReadAnimationJoint(std::size_t& offset, char* fileData, const unsigned int& fileByteSize)
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

void RawModelCustom::ReadAnimationClips(std::size_t& offset, char* fileData, const unsigned int& fileByteSize, unsigned int numberOfClips)
{
    for (unsigned int i = 0; i < numberOfClips; i++) {
        ReadAnimationClipSingle(offset, fileData, fileByteSize, i);
    }
}

void RawModelCustom::ReadAnimationClipSingle(std::size_t& offset, char* fileData, const unsigned int& fileByteSize, unsigned int clipIndex)
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
        throw Resource::FailedLoadingException("Reading AnimationClip numberOfJointFrames failed");
    }
    unsigned int numberOfJointFrames = *(unsigned int*)(fileData + offset);
    offset += sizeof(unsigned int);

    for (unsigned int i = 0; i < numberOfJointFrames; i++) {
        if (offset + sizeof(unsigned int) > fileByteSize) {
            throw Resource::FailedLoadingException("Reading AnimationClip JointID failed");
        }
        int jointID = *(unsigned int*)(fileData + offset);
        offset += sizeof(unsigned int);

        if (offset + sizeof(unsigned int) > fileByteSize) {
            throw Resource::FailedLoadingException("Reading AnimationClip numberOFKeyFrames failed");
        }
        unsigned int numberOFKeyFrames = *(unsigned int*)(fileData + offset);
        offset += sizeof(unsigned int);

        if (numberOFKeyFrames > 0) {
            newAnimation.JointAnimations[jointID].reserve(numberOFKeyFrames);

            for (unsigned int j = 0; j < numberOFKeyFrames; j++) {
                ReadAnimationKeyFrame(offset, fileData, fileByteSize, newAnimation.JointAnimations[jointID]);
            }
        }
    }
    m_Skeleton->Animations[newAnimation.Name] = newAnimation;
    //m_Skeleton->Animations[newAnimation.Name].KeyFrameAmount = nrOfKeyframes;
#else
#endif
}

void RawModelCustom::ReadAnimationKeyFrame(std::size_t& offset, char* fileData, const unsigned int& fileByteSize, std::vector<Skeleton::Animation::Keyframe>& animation)
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

    if (offset + sizeof(float) * 3 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationKeyFrame Position failed");
    }
    memcpy(&newKeyFrame.BoneProperties.Position[0], fileData + offset, sizeof(float) * 3);
    offset += sizeof(float) * 3;

    if (offset + sizeof(float) * 4 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationKeyFrame Rotation failed");
    }
    memcpy(&newKeyFrame.BoneProperties.Rotation[0], fileData + offset, sizeof(float) * 4);
    offset += sizeof(float) * 4;

    if (offset + sizeof(float) * 3 > fileByteSize) {
        throw Resource::FailedLoadingException("Reading AnimationKeyFrame Scale failed");
    }
    memcpy(&newKeyFrame.BoneProperties.Scale[0], fileData + offset, sizeof(float) * 3);
    offset += sizeof(float) * 3;


    animation.push_back(newKeyFrame);

}

RawModelCustom::~RawModelCustom()
{
    // Ownership of skeleton and materials get transferred to Model
 //   if (m_Skeleton != nullptr) {
 //       delete m_Skeleton;
 //   }
	//for (auto material : m_Materials) {
	//	delete material.material;
	//}
}

#endif