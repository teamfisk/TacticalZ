#include "Rendering/ShadowPassState.h"

ShadowPassState::ShadowPassState(GLuint frameBuffer)
{
    GLERROR("---2");
    BindFramebuffer(frameBuffer);
    GLERROR("---3");
    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);
    Disable(GL_BLEND);
}

ShadowPassState::~ShadowPassState()
{

}