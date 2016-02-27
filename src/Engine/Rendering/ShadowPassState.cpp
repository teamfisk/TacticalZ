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
}

ShadowPassState::~ShadowPassState()
{

}