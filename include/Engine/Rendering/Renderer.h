#ifndef Renderer_h__
#define Renderer_h__

#include <sstream>

#include "IRenderer.h"
#include "ShaderProgram.h"
//TODO: Temp resourceManager
#include "../Core/ResourceManager.h"

class Renderer : public IRenderer
{
public:
	virtual void Initialize() override;
	virtual void Draw(RenderQueueCollection& rq) override;

private:
	//----------------------Variables----------------------//
	RenderQueueCollection m_TempRQ;
	Texture* m_ErrorTexture;

	//----------------------Functions----------------------//
	void InitializeWindow();
	void InitializeShaders();
	//TODO: Render: Remove ModelsToDraw from Renderer.
	void ModelsToDraw();
	void EnqueueModel(Model* model);

	//--------------------ShaderPrograms-------------------//
	ShaderProgram m_BasicForwardProgram;
};

#endif