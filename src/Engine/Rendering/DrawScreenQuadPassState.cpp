#include "Rendering/DrawScreenQuadPassState.h"


DrawScreenQuadPassState::DrawScreenQuadPassState()
{
    GLERROR("---");
    BindFramebuffer(0);
    GLERROR("---");
    Disable(GL_DEPTH_TEST);
    Disable(GL_CULL_FACE);
    Disable(GL_BLEND);
    ClearColor(glm::vec4(0.f));
}

DrawScreenQuadPassState::~DrawScreenQuadPassState()
{

}
