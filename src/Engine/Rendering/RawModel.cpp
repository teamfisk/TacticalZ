#include "Rendering/RawModel.h"

RawModel::RawModel(std::string fileName)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(fileName, aiProcess_CalcTangentSpace | aiProcess_Triangulate);

	if (scene == nullptr) {
		LOG_ERROR("Failed to load model \"%s\"", fileName.c_str());
		LOG_ERROR("Assimp error: %s", importer.GetErrorString());
        throw std::runtime_error("Failed to open model file.");
	}

	auto m = scene->mRootNode->mTransformation;
	m_Matrix = glm::mat4(
		m.a1, m.a2, m.a3, m.a4,
		m.b1, m.b2, m.b3, m.b4,
		m.c1, m.c2, m.c3, m.c4,
		m.d1, m.d2, m.d3, m.d4
	);
	m_Matrix = glm::transpose(m_Matrix);

	auto meshes = scene->mMeshes;

	// Pre-count vertices
	int numVertices = 0;
	int numIndices = 0;
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		numVertices += meshes[i]->mNumVertices;

		// Faces
		for (int j = 0; j < meshes[i]->mNumFaces; ++j) {
			auto face = meshes[i]->mFaces[j];
			numIndices += face.mNumIndices;
		}
	}
	//LOG_DEBUG("Vertex count %i", numVertices);
	//LOG_DEBUG("Index count %i", numIndices);

	//LOG_DEBUG("Model has %i embedded textures", scene->mNumTextures);

	std::vector<std::tuple<std::string, glm::mat4>> boneInfo;
	std::map<std::string, int> boneNameMapping;

	for (int i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = meshes[i];
		auto material = scene->mMaterials[mesh->mMaterialIndex];
		unsigned int indexOffset = m_Vertices.size();

		// Vertices, normals and texture coordinates
		for (int vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			Vertex desc;

			// Position
			auto position = mesh->mVertices[vertexIndex];
			desc.Position = glm::vec3(position.x, position.y, position.z);

			// Normal
			auto normal = mesh->mNormals[vertexIndex];
			desc.Normal = glm::vec3(normal.x, normal.y, normal.z);

			//if (mesh->HasTangentsAndBitangents()) {
			//	// Tangent
			//	auto tangent = mesh->mTangents[vertexIndex];
			//	desc.Tangent = glm::vec3(tangent.x, tangent.y, tangent.z);

			//	// Bi-tangent
			//	auto bitangent = mesh->mBitangents[vertexIndex];
			//	desc.BiTangent = glm::vec3(bitangent.x, bitangent.y, bitangent.z);
			//}

			// UV
			if (mesh->HasTextureCoords(0)) {
				auto uv = mesh->mTextureCoords[0][vertexIndex];
				desc.TextureCoords = glm::vec2(uv.x, uv.y);
			}

			// Material diffuse color
			aiColor3D diffuse;
			material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            float opacity;
            material->Get(AI_MATKEY_OPACITY, opacity);
			desc.DiffuseVertexColor = glm::vec4(diffuse.r, diffuse.g, diffuse.b, opacity);

			desc.DiffuseVertexColor = glm::vec4(diffuse.r, diffuse.g, diffuse.b, opacity);
            
			// Material specular color
			aiColor3D specular;
			material->Get(AI_MATKEY_COLOR_SPECULAR, specular);
			desc.SpecularVertexColor = glm::vec4(specular.r, specular.g, specular.b, 1.f);

			m_Vertices.push_back(desc);
		}

		// Faces
		for (int j = 0; j < mesh->mNumFaces; ++j) {
			auto face = mesh->mFaces[j];
			for (int k = 0; k < face.mNumIndices; ++k) {
				unsigned int index = face.mIndices[k];
				m_Indices.push_back(indexOffset + index);
			}
		}

		// Calculate normal mapping tangents
		for (int i = 0; i < m_Indices.size(); i += 3) {
			Vertex& v0 = m_Vertices[m_Indices[i]];
			Vertex& v1 = m_Vertices[m_Indices[i + 1]];
			Vertex& v2 = m_Vertices[m_Indices[i + 2]];

			glm::vec3 edge1 = v1.Position - v0.Position;
			glm::vec3 edge2 = v2.Position - v0.Position;

			float deltaU1 = v1.TextureCoords.x - v0.TextureCoords.x;
			float deltaV1 = v1.TextureCoords.y - v0.TextureCoords.y;
			float deltaU2 = v2.TextureCoords.x - v0.TextureCoords.x;
			float deltaV2 = v2.TextureCoords.y - v0.TextureCoords.y;

			float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

			glm::vec3 tangent;
			tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
			tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
			tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

			v0.Tangent += tangent;
			v1.Tangent += tangent;
			v2.Tangent += tangent;
		}
		for (auto& vertex : m_Vertices) {
			vertex.Tangent = glm::normalize(vertex.Tangent);
			vertex.BiTangent = glm::normalize(glm::cross(vertex.Tangent, glm::normalize(vertex.Normal)));
		}

		// Material info
		MaterialGroup matGroup;
		matGroup.StartIndex = indexOffset;
		matGroup.EndIndex = m_Indices.size() - 1;
		// Material shininess
		material->Get(AI_MATKEY_SHININESS, matGroup.Shininess);
        material->Get(AI_MATKEY_OPACITY, matGroup.Transparency);
		//LOG_DEBUG("Shininess: %f", matGroup.Shininess);
		// Diffuse texture
		//LOG_DEBUG("%i diffuse textures found", material->GetTextureCount(aiTextureType_DIFFUSE));
		if (material->GetTextureCount(aiTextureType_DIFFUSE)) {
			aiString path;
			aiTextureMapping mapping;
			material->GetTexture(aiTextureType_DIFFUSE, 0, &path, &mapping);
			std::string absolutePath = (boost::filesystem::path(fileName).branch_path() / path.C_Str()).string();
			//LOG_DEBUG("Diffuse texture: %s", absolutePath.c_str());
			matGroup.Texture = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(absolutePath));
		}
		// Normal map
		//LOG_DEBUG("%i normal maps found", material->GetTextureCount(aiTextureType_HEIGHT));
		if (material->GetTextureCount(aiTextureType_HEIGHT)) {
			aiString path;
			aiTextureMapping mapping;
			material->GetTexture(aiTextureType_HEIGHT, 0, &path, &mapping);
			std::string absolutePath = (boost::filesystem::path(fileName).branch_path() / path.C_Str()).string();
			//LOG_DEBUG("Normal map: %s", absolutePath.c_str());
			matGroup.NormalMap = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(absolutePath));
		}
		// Specular map
		//LOG_DEBUG("%i specular maps found", material->GetTextureCount(aiTextureType_SPECULAR));
		if (material->GetTextureCount(aiTextureType_SPECULAR)) {
			aiString path;
			aiTextureMapping mapping;
			material->GetTexture(aiTextureType_SPECULAR, 0, &path, &mapping);
			std::string absolutePath = (boost::filesystem::path(fileName).branch_path() / path.C_Str()).string();
			//LOG_DEBUG("Specular map: %s", absolutePath.c_str());
			matGroup.SpecularMap = std::shared_ptr<Texture>(ResourceManager::Load<Texture>(absolutePath));
		}
		TextureGroups.push_back(matGroup);

		// Bones
		std::map<int, std::vector<std::tuple<int, float>>> vertexWeights;
		for (int j = 0; j < mesh->mNumBones; ++j) {
			auto bone = mesh->mBones[j];
			std::string boneName = bone->mName.C_Str();

			auto mat = bone->mOffsetMatrix;
			glm::mat4 glmMat(mat.a1, mat.b1, mat.c1, mat.d1,
				mat.a2, mat.b2, mat.c2, mat.d2,
				mat.a3, mat.b3, mat.c3, mat.d3,
				mat.a4, mat.b4, mat.c4, mat.d4);

			int boneIndex;
			if (boneNameMapping.find(boneName) != boneNameMapping.end()) {
				boneIndex = boneNameMapping[boneName];
			} else {
				boneIndex = boneInfo.size();
				boneInfo.push_back(std::make_tuple(boneName, glmMat));
				boneNameMapping[boneName] = boneIndex;
			}

			for (int k = 0; k < bone->mNumWeights; ++k) {
				auto weight = bone->mWeights[k];
				unsigned int offsetVertexId = weight.mVertexId + indexOffset;
				vertexWeights[offsetVertexId].push_back(std::make_tuple(boneIndex, weight.mWeight));
			}
		}
		for (auto &pair : vertexWeights) {
			auto weights = pair.second;
			Vertex& desc = m_Vertices[pair.first];

			const int maxWeights = 8;
			if (weights.size() > maxWeights) {
				LOG_WARNING("Vertex weights (%i) greater than max weights per vertex (%i)", weights.size(), maxWeights);
			}
			for (int weightIndex = 0; weightIndex < weights.size() && weightIndex < maxWeights && weightIndex < 4; ++weightIndex) {
				std::tie(desc.BoneIndices1[weightIndex], desc.BoneWeights1[weightIndex]) = weights[weightIndex];
			}
			for (int weightIndex = 4; weightIndex < weights.size() && weightIndex < maxWeights && weightIndex < 8; ++weightIndex) {
				std::tie(desc.BoneIndices2[weightIndex - 4], desc.BoneWeights2[weightIndex - 4]) = weights[weightIndex];
			}
		}

		//break;
	}

	// Traverse the node tree and build a skeleton
	if (!boneInfo.empty()) {
		m_Skeleton = new Skeleton();
		CreateSkeleton(boneInfo, boneNameMapping, scene->mRootNode, -1);
		int numBones = m_Skeleton->Bones.size();
		//LOG_DEBUG("Bone count: %i", numBones);
		if (numBones > 0) {
			m_Skeleton->PrintSkeleton();
		}
	}

	// Animations
	//LOG_DEBUG("Animation count: %i", scene->mNumAnimations);
	for (int i = 0; i < scene->mNumAnimations; ++i) {
		auto animation = scene->mAnimations[i];
		std::string animationName = animation->mName.C_Str();
		//LOG_DEBUG("Animation: %s", animationName.c_str());
		//LOG_DEBUG("Duration: %f", animation->mDuration);
		//LOG_DEBUG("Ticks per second: %f", animation->mTicksPerSecond);

		Skeleton::Animation skelAnim;
		skelAnim.Name = animationName;
		skelAnim.Duration = animation->mDuration / animation->mTicksPerSecond;

		std::map<int, double> frameTimes;
		std::map<int, std::map<int, Skeleton::Animation::Keyframe::BoneProperty>> frameBoneProperties;
		// For each animation channel (bone)
		for (int channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
			auto channel = animation->mChannels[channelIndex];
			std::string boneName = channel->mNodeName.C_Str();
			int boneID = m_Skeleton->GetBoneID(boneName);
			if (boneID == -1) {
				LOG_ERROR("Animation referenced a bone that doesn't exist: %s", boneName.c_str());
				continue;
			}

			// If you don't have the same amount of keyframes for every transformation type you're dumb.
			if (channel->mNumPositionKeys != channel->mNumRotationKeys || channel->mNumPositionKeys != channel->mNumScalingKeys) {
				LOG_ERROR("Hey, animation! You're dumb!", animationName.c_str());
				continue;
			}

			for (int keyframe = 0; keyframe < channel->mNumPositionKeys; ++keyframe) {
				auto posKey = channel->mPositionKeys[keyframe];
				auto rotKey = channel->mRotationKeys[keyframe];
				auto scaleKey = channel->mScalingKeys[keyframe];

				frameTimes[keyframe] = posKey.mTime;

				auto &property = frameBoneProperties[keyframe][boneID];
				property.ID = keyframe;
				property.Position = glm::vec3(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
				property.Rotation = glm::quat(rotKey.mValue.w, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z);
				property.Scale = glm::vec3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);
			}
		}

		// Create keyframes from bone properties
		for (auto &kv : frameBoneProperties) {
			int keyframe = kv.first;
			Skeleton::Animation::Keyframe animationFrame;
			animationFrame.Index = keyframe;
			animationFrame.Time = frameTimes[keyframe] / animation->mTicksPerSecond;
			// HACK: For some reason Blender likes to create a first frame that doesn't start at time 0
			if (keyframe == 0) {
				animationFrame.Time = 0;
			}
			for (auto &kv2 : kv.second) {
				int boneID = kv2.first;
				auto &property = kv2.second;
				animationFrame.BoneProperties[boneID] = property;
			}
			skelAnim.Keyframes.push_back(animationFrame);
		}

		m_Skeleton->Animations[animationName] = skelAnim;
	}
}

RawModel::~RawModel()
{
	if (m_Skeleton) {
		delete m_Skeleton;
	}
}

void RawModel::CreateSkeleton(std::vector<std::tuple<std::string, glm::mat4>> &boneInfo, std::map<std::string, int> &boneNameMapping, aiNode* node, int parentID)
{
	std::string nodeName = node->mName.C_Str();

	// Find the bone by name in the bone info list
	if (boneNameMapping.find(nodeName) == boneNameMapping.end()) {
		//LOG_DEBUG("Node \"%s\" was not a bone", nodeName.c_str());
	} else {
		glm::mat4 offsetMatrix;
		int ID = boneNameMapping[nodeName];
		std::tie(std::ignore, offsetMatrix) = boneInfo[ID];
		m_Skeleton->CreateBone(ID, parentID, nodeName, offsetMatrix);
		parentID = ID;
	}

	for (int childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		aiNode* child = node->mChildren[childIndex];
		CreateSkeleton(boneInfo, boneNameMapping, child, parentID);
	}
}
