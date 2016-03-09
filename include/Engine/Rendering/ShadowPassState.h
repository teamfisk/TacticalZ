#ifndef ShadowPassState_h_
#define ShadowPassState_h_

#include "Rendering/RenderState.h"

class ShadowPassState : public RenderState
{
public:
	ShadowPassState(GLuint frameBuffer);
	~ShadowPassState();

private:
};

#endif