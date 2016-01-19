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
#include "TextJob.h"
#include "PointLightJob.h"
#include "DirectionalLightJob.h"

struct RenderScene
{
    ::Camera* Camera;
    std::list<std::shared_ptr<RenderJob>> ForwardJobs;
    std::list<std::shared_ptr<RenderJob>> PointLightJobs;
    std::list<std::shared_ptr<RenderJob>> TextJobs;
    std::list<std::shared_ptr<RenderJob>> DirectionalLightJobs;
    Rectangle Viewport;

	void Clear()
	{
        ForwardJobs.clear();
        PointLightJobs.clear();
        TextJobs.clear();
        DirectionalLightJobs.clear();
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