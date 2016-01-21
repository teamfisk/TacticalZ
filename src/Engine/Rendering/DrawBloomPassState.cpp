#include "Rendering/DrawBloomPassState.h"


DrawBloomPassState::DrawBloomPassState()
{
    //BindFramebuffer(0);
    Disable(GL_BLEND);
    Disable(GL_DEPTH_TEST);
    Disable(GL_CULL_FACE);
    ClearColor(glm::vec4(0.f / 255, 0.f / 255, 0.f / 255, 0.f));
    Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

DrawBloomPassState::~DrawBloomPassState()
{

}
