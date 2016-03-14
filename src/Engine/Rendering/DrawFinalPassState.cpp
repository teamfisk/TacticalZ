#include "Rendering/DrawFinalPassState.h"



DrawFinalPassState::DrawFinalPassState(GLuint frameBuffer)
{
    BindFramebuffer(frameBuffer);
    Enable(GL_BLEND);
    BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Enable(GL_DEPTH_TEST);
	DepthMask(GL_TRUE);
    Enable(GL_CULL_FACE);
	Enable(GL_ALPHA_TEST);
	AlphaFunc(GL_GEQUAL, 0.05f);
   // Enable(GL_STENCIL_TEST);
   // StencilFunc(GL_NOTEQUAL, 1, 0xFF);
   // StencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
   // StencilMask(0xFF);
    ClearColor(glm::vec4(0.f, 0.f, 0.f, 0.f));
}

DrawFinalPassState::~DrawFinalPassState()
{

}

DrawStencilState::DrawStencilState(GLuint frameBuffer)
{
    BindFramebuffer(frameBuffer);
    Enable(GL_DEPTH_TEST);
	DepthMask(GL_TRUE);
	Enable(GL_CULL_FACE);
    ClearColor(glm::vec4(0.f));
}

DrawStencilState::~DrawStencilState()
{

}

