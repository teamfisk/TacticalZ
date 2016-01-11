#include "Rendering/DrawFinalPassState.h"


DrawFinalPassState::DrawFinalPassState()
{
    BindFramebuffer(0);

    Enable(GL_DEPTH_TEST);
    Enable(GL_CULL_FACE);
    ClearColor(glm::vec4(200.f / 255, 0.f / 255, 200.f / 255, 0.f));
    Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

DrawFinalPassState::~DrawFinalPassState()
{

}
