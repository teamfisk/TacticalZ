#include "Rendering/DrawFinalPassState.h"


DrawFinalPassState::DrawFinalPassState(GLuint frameBuffer)
{
    BindFramebuffer(frameBuffer);
    Enable(GL_BLEND);
    BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);
    ClearColor(glm::vec4(0.f, 0.f, 0.f, 0.f));
}

DrawFinalPassState::~DrawFinalPassState()
{

}
