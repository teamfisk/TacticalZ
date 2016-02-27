#include "Rendering/ShadowPassState.h"

ShadowPassState::ShadowPassState(GLuint frameBuffer)
{
    GLERROR("---2");
    BindFramebuffer(frameBuffer);
    GLERROR("---3");
    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);
    Disable(GL_BLEND);
	Disable(GL_TEXTURE_2D);
	CullFace(GL_FRONT);
	ClearColor(glm::vec4(255.f, 128.f, 128.f, 128.f));
	GLERROR("---4");
}

ShadowPassState::~ShadowPassState()
{

}