#ifndef RenderQueue_h__
#define RenderQueue_h__

#include <cstdint>
#include <forward_list>

#include "../Common.h"
#include "../GLM.h"
#include "../Core/Util/Rectangle.h"
#include "../Core/Entity.h"
#include "Camera.h"
#include "RenderJob.h"
#include "ModelJob.h"



/*


struct TransparentModelJob : RenderJob
{
    unsigned int ShaderID = 0;
    unsigned int TextureID = 0;

    //TODO: RENDERER: Not sure if the best solution for pickingColor to entity link is this
    EntityID Entity;

    glm::mat4 Matrix;
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
*/

struct RenderScene
{
    ::Camera* Camera;
    std::list<std::shared_ptr<RenderJob>> ForwardJobs;
    std::list<std::shared_ptr<RenderJob>> LightJobs;
    Rectangle ViewPort;

	void Clear()
	{
        ForwardJobs.clear();
        LightJobs.clear();
	}
};

struct RenderFrame
{
public:

    void Add(RenderScene &scene)
    {
        RenderScenes.push_back(std::shared_ptr<RenderScene>(new RenderScene(scene)));
        m_Size++;
    }

    void Clear()
    {
        RenderScenes.clear();
        m_Size = 0;
    }

    int Size() const { return m_Size; }
    std::list<std::shared_ptr<RenderScene>>::const_iterator begin()
    {
        return RenderScenes.begin();
    }

    std::list<std::shared_ptr<RenderScene>>::const_iterator end()
    {
        return RenderScenes.end();
    }

    std::list<std::shared_ptr<RenderScene>> RenderScenes;
    
private:
    int m_Size = 0;
};

#endif