#include "Rendering/SSAOPassState.h"


SSAOPassState::SSAOPassState()
{
	//BindFramebuffer(0);
	Disable(GL_BLEND);
	Disable(GL_DEPTH_TEST);
	Disable(GL_CULL_FACE);
}

SSAOPassState::~SSAOPassState()
{

}

