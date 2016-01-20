#include "Rendering/DrawFinalPassState.h"


DrawFinalPassState::DrawFinalPassState()
{
    //BindFramebuffer(0);
    Enable(GL_BLEND);
    BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);
    ClearColor(glm::vec4(155.f / 255, 0.f / 255, 155.f / 255, 0.f));
    Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

DrawFinalPassState::~DrawFinalPassState()
{

}
