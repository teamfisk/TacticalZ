#ifndef Renderer_h__
#define Renderer_h__

#include <sstream>

#include "IRenderer.h"
#include "ShaderProgram.h"

class Renderer : public IRenderer
{
public:
	virtual void Initialize() override;
	virtual void Draw(RenderQueueCollection& rq) override;

private:
	void InitializeWindow();
	void InitializeShaders();


	ShaderProgram m_BasicForwardProgram;
};

#endif