#include "Rendering/ShadowPassState.h"

ShadowPassState::ShadowPassState(GLuint frameBuffer)
{
	BindFramebuffer(frameBuffer);
	Enable(GL_DEPTH_TEST);
	Enable(GL_CULL_FACE);
	Disable(GL_BLEND);
	Disable(GL_TEXTURE_2D);
	CullFace(GL_FRONT);
	ClearColor(glm::vec4(0.f, 0.f, 0.f, 0.f));
	//Enable(GL_ALPHA_TEST);
	//glAlphaFunc(GL_GREATER, 0.9f);
}

ShadowPassState::~ShadowPassState()
{

}