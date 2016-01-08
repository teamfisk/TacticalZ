#ifndef RenderQueue_h__
#define RenderQueue_h__

#include <cstdint>
#include <forward_list>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/Util/Rectangle.h"
#include "../Core/Entity.h"

class Model;
class Skeleton;
class Texture;
class RenderQueue;

//TODO: Render: Remove obsolete RenderJobs and fix standard values on variables.

struct RenderJob
{
	friend class RenderQueue;

	float Depth;

protected:
	uint64_t Hash;

	virtual void CalculateHash() = 0;

	bool operator<(const RenderJob& rhs)
	{
		return this->Hash < rhs.Hash;
	}
};

struct ModelJob : RenderJob
{
	unsigned int ShaderID = 0;
	unsigned int TextureID = 0;

    //TODO: RENDERER: Not sure if the best solution for pickingColor to entity link is this
    EntityID Entity;

	glm::mat4 ModelMatrix;
	const Texture* DiffuseTexture;
	const Texture* NormalTexture;
	const Texture* SpecularTexture;
	float Shininess = 0.f;
	glm::vec4 Color;
	const Model* Model = nullptr;
	unsigned int StartIndex = 0;
	unsigned int EndIndex = 0;

	// Animation
	Skeleton* Skeleton = nullptr;
	bool NoRootMotion = true;
	std::string AnimationName;
	double AnimationTime = 0;

	void CalculateHash() override
	{
		Hash = TextureID;
	}
};

// Cool Death Animation
struct CoolDeathAnimationJob : ModelJob
{
    glm::vec3 OriginPos;
    float TimeSinceDeath = 0.f;
    float EndOfDeath = 15.0f;

};



struct SpriteJob : RenderJob
{
	unsigned int ShaderID = 0;
	unsigned int TextureID = 0;

	glm::mat4 ModelMatrix;
	const Texture* DiffuseTexture = nullptr;
	const Texture* NormalTexture = nullptr;
	const Texture* SpecularTexture = nullptr;
	glm::vec4 Color;

	void CalculateHash() override
	{
		Hash = TextureID;
	}
};

struct PointLightJob : RenderJob
{
	glm::vec3 Position;
	glm::vec3 SpecularColor = glm::vec3(1, 1, 1);
	glm::vec3 DiffuseColor = glm::vec3(1, 1, 1);
	float Radius = 1.f;
    float Intensity = 0.8f;

	void CalculateHash() override
	{
		Hash = 0;
	}
};

class RenderQueue
{
public:
	template <typename T>
	void Add(T &job)
	{
		job.CalculateHash();
		Jobs.push_back(std::shared_ptr<T>(new T(job)));
		m_Size++;
	}

	void Sort()
	{
		Jobs.sort();
	}

	void Clear()
	{
		Jobs.clear();
		m_Size = 0;
	}

	int Size() const { return m_Size; }
	std::list<std::shared_ptr<RenderJob>>::const_iterator begin()
	{
		return Jobs.begin();
	}

	std::list<std::shared_ptr<RenderJob>>::const_iterator end()
	{
		return Jobs.end();
	}

	std::list<std::shared_ptr<RenderJob>> Jobs;

private:
	int m_Size = 0;
};

struct RenderQueueCollection
{
	RenderQueue Forward;
	RenderQueue Lights;

	void Clear()
	{
		Forward.Clear();
		Lights.Clear();
	}

	void Sort()
	{
		Forward.Sort();
		Lights.Sort();
	}
};

#endif