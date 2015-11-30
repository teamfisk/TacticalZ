#ifndef DummyRenderer_h__
#define DummyRenderer_h__

#include <sstream>

#include "IRenderer.h"

class DummyRenderer : public IRenderer
{
public:
	virtual void Initialize() override;
	virtual void Draw(RenderQueueCollection& rq) override;
};

#endif